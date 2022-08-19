#include "metascheduler.hpp"

extern Fractal::MsGraph msg;
extern Fractal::Calendar cal;

static int scheduleSequence_nr = 0;
extern int reconvHorizon;
extern int samp_freq;
int adaptation_sz = 1;
extern int minx;

extern int a,b;
extern float c,d,e, average;

void createCalendar(Fractal::Schedule *schedule, Fractal::Context *CM) {
	//Copy events in CM to the calendar
	for (Fractal::Event E : CM->events)
		cal.addEvent(E);

	//For each event in calendar, update event time
	cal.updateEvents(schedule);

	//Sort the calendar by time of event
	cal.sortCalendar();

	//print contents of calendar
	//cal.printCalendar();
}

void reorderCalendar(Fractal::Event *E, Fractal::Calendar *cal, Fractal::Schedule *schedule) {
	if (cal->getEvents().empty())
		return;

	//copy non-mutually exclusive events in calendar to temporary list
	list <Fractal::Event> temp;
	for (Fractal::Event event : cal->getEvents()) {
		if (event.target != E->target || event.eventType != E->eventType) {
			temp.push_back(event);
		}
	}

	//remove all objects from previous calendar and copy contents of temporary calendar to previous
	cal->clearCalendar();
	for (Fractal::Event event : temp) {
		cal->addEvent(event);
	}

	//cal->printCalendar();

	//reorder mutually exclusive calendar
	cal->updateEvents(schedule);

	//Sort the calendar by time of event
	cal->sortCalendar();
	//delete &temp;
}

Fractal::Model apply_event(Fractal::Event* E, Fractal::Model model) {
	//modify AM, PM based on event E
	switch (E->eventType) {
	case SLACK:
		E->old = model.job(E->target).wcet_fullspeed;
		model.job(E->target).wcet_fullspeed = E->value;
		break;

	case FAULT:
		model.remove_node(model.node(E->target).id);
		break;
	}
	return model;
}

Fractal::Job slack_job (Fractal::Schedule base, int event_time) {
	vector<Fractal::Job> slack_job;

	for_each (base.jobs().begin(), base.jobs().end(), [&slack_job, &event_time] (Fractal::Job& j){
		if (event_time > j.start_time and event_time < (j.start_time+j.wcet_fullspeed)) {
			slack_job.push_back(j);
		}
	});

	if (slack_job.empty()) {
		Fractal::Job job = base.job(0);
		job.id = -1;
		return job;
	}

	sort (slack_job.begin(), slack_job.end(), [&event_time](Fractal::Job& lhs, Fractal::Job&rhs) {
		return ((lhs.start_time+lhs.wcet_fullspeed) - event_time) > ((rhs.start_time+rhs.wcet_fullspeed) - event_time);
	});

	return slack_job.front();
}

void slack_metaschedule (Fractal::Schedule base, list<int> sample_times, Fractal::Model model) {
	if (sample_times.empty()) {
		return;
	}

	int event_time = sample_times.front();
	sample_times.pop_front();

	slack_metaschedule (base, sample_times, model);

	if (event_time >= base.makespan()) {
		return;
	}

	Fractal::Job job = slack_job (base, event_time);
	if (job.id == -1) {
		return;
	}
	model.job(job.id).wcet_fullspeed = event_time;
	model.horizonStart = event_time;

	int start = job.start_time;
	int end = job.start_time+job.wcet_fullspeed;

	Fractal::Event e (job.id);
	e.eventTime = event_time;
	e.eventType = SLACK;
	e.old = job.wcet_fullspeed;
	e.value = ((float)((end-e.eventTime)/job.wcet_fullspeed))*100;

	Fractal::Horizon h ((event_time+adaptation_sz), numeric_limits<int>::max());
	Fractal::scheduler_config crashconfig = {1000, 10000, 0.01, 0.4, 1.0};
	model.horizonStart = 0;

	Fractal::Scheduler activeScheduler(&model, &crashconfig, &h);
	activeScheduler.evolve();
	//activeScheduler.evolve();

	Fractal::Schedule *Schedule = new Fractal::Schedule(activeScheduler.best());

	if (Schedule->makespan() > base.makespan()) {
		Schedule->jobs() = base.jobs();
		Schedule->messages() = base.messages();
		Schedule->nodes() = base.nodes();
		Schedule->links() = base.links();
		Schedule->set_makespan(base.makespan());
	}
	cout << "New schedule makespan: " << Schedule->makespan() << endl;

	Schedule->set_uid(scheduleSequence_nr++);
	Schedule->setParent(base.uid());
	//Schedule->export_json("results/", e);

	slack_metaschedule (*Schedule, sample_times, model);
}

void dirty_metaschedule (Fractal::Schedule base, int sample_freq, Fractal::Model model) {
	list <int> sample_time;
	int event_t = base.makespan()/(sample_freq+1);
	for (int i=1; i<=sample_freq; i++) {
		sample_time.push_back(event_t*i);
	}

/*	cout << "Schedule makespan: " << base.makespan() << endl;*/
	for (auto time : sample_time) {
		cout << "Sample points: " << time << endl;
	}

	slack_metaschedule (base, sample_time, model);
}

void newSlack_meta(Fractal::Model model, list <int> sample_times, Fractal::MsgNode* parent, Fractal::Calendar cal) {
	if (sample_times.empty() || cal.getEvents().empty()) {
		return;
	}

	int event_time = sample_times.front();
	sample_times.pop_front();
	vector <Fractal::Event> events;
	for (auto& e : cal.getEvents()) {
		if (e.eventTime<event_time) {
			e.old = model.job(e.target).wcet_fullspeed;
			events.push_back(e);
			cal.removeEvent(e.ID);
		}
	}

	newSlack_meta(model, sample_times, parent, cal);

	if (events.empty()) {
		return;
	}
	model.apply_events(events);

/*	for (auto a : model.jobs()) {
		if (a.wcet_fullspeed<8)
		cout << a.wcet_fullspeed << endl;
	}*/

	const int reconv = reconvHorizon;
	model.horizonStart = event_time+1;
	model.horizonEnd = (parent->getSchedule()->makespan()+1);

	Fractal::Horizon horizon (model.horizonStart, model.horizonEnd);
	Fractal::scheduler_config crashconfig = {100, 100, 0.2, 0.9, 1.0};
	Fractal::Scheduler activeScheduler(&model, &crashconfig, &horizon);
		activeScheduler.evolve();
		model.zu = 1;
	Fractal::Schedule *Schedule = new Fractal::Schedule(activeScheduler.best());
	model.zu = 0;
	if (Schedule->makespan() > parent->getSchedule()->makespan()) {
		Schedule->jobs() = parent->getSchedule()->jobs();
		Schedule->messages() = parent->getSchedule()->messages();
		Schedule->nodes() = parent->getSchedule()->nodes();
		Schedule->links() = parent->getSchedule()->links();
		Schedule->set_makespan(parent->getSchedule()->makespan());
	}

	Schedule->set_uid(scheduleSequence_nr++);
	Schedule->setParent(parent->getSchedule()->uid());
	Schedule->export_json("results/", events);

	//create new node and add key (schedule)
	Fractal::MsgNode *Node = new Fractal::MsgNode(Schedule->uid(), Schedule, events);
	parent->addChild(Node);

	cout << "New schedule: " << Schedule->uid() << ", Makespan: " << Schedule->makespan() << endl;

	minx = min(minx, Schedule->makespan());

	cal.updateEvents(Schedule);
	cal.sortCalendar();
	Node->saveCalendar(&cal);
	msg.addNode(Node);

	newSlack_meta(model,sample_times, Node, cal);
}

void n_metaschedule (Fractal::Model model, int sample_freq, Fractal::MsgNode* base, Fractal::Calendar cal) {
	//model.sort_js();

	list <int> sample_times;
	//int event_t = model.jobs().front().wcet_fullspeed/sample_freq;
	for (int i=sample_freq; i<base->getSchedule()->makespan(); i+=sample_freq) {
			cout << i << ",";
			sample_times.push_back(i);
	}
	cout << endl;
	//model.unsort_js();

	cout << "Number of Sample Points: " << sample_times.size() << endl;

	newSlack_meta(model, sample_times, base, cal);
	cout << "Minimum Makespan: " << minx << endl;

	int baseM = base->getSchedule()->makespan();
	float energySaving = ((float)(baseM-minx)/(float)baseM)*100;
	cout << "Energy Saved: " << energySaving << endl;
	cout << "Schedule Energy: " << ((float)minx/(float)baseM)*100 << endl;
	average = average + energySaving;
}

void metaSchedule(Fractal::Model model, Fractal::Calendar cal, Fractal::MsgNode *prev) {
	//end recursion
	if (cal.getEvents().empty())
		return;

	//Take earliest event and remove from calendar
	Fractal::Event E = cal.getEvents().front();
	cal.removeEvent(E.ID);

	//recursion
	metaSchedule(model, cal, prev);

	//apply event E to change AM, PM
	Fractal::Model newModel = apply_event(&E, model);

	//get time unit for re-convergence horizon
	const int reconv = reconvHorizon;
	newModel.horizonStart = E.eventTime;

	//set reconvergence window for re-scheduling
	Fractal::Horizon *horizon = new Fractal::Horizon(E.eventTime, (prev->getSchedule()->makespan()+1));

	//initialize crash config needed by the GA
	Fractal::scheduler_config crashconfig = {100, 1000, 0.2, 0.9, 1.0};

	//auto t1 = high_resolution_clock::now();
	//invoke GA to obtain root schedule
	Fractal::Scheduler activeScheduler(&newModel, &crashconfig, horizon);
	activeScheduler.evolve();
	model.zu = 1;

	Fractal::Schedule *Schedule = new Fractal::Schedule(activeScheduler.best());
	model.zu = 0;

	if (Schedule->makespan() > prev->getSchedule()->makespan()) {
		Schedule = prev->getSchedule();
	}

/*	auto t2 = high_resolution_clock::now();

	 //Getting number of milliseconds as an integer.
	    auto ms_int = duration_cast<milliseconds>(t2 - t1);

	    cout << "Time to compute New Schedule: " << ms_int.count() << "ms\n";*/

	//Set new schedule ID
	Schedule->set_uid(scheduleSequence_nr++);
	Schedule->setParent(prev->getID());
	vector<Fractal::Event> events={E};
	//Export new schedule to json file.
	Schedule->export_json("results/", events);

	//create new node and add key (schedule)
	Fractal::MsgNode *Node = new Fractal::MsgNode(Schedule->uid(), Schedule, events);
	prev->addChild(Node);

	cout << "New Node: " << Node->getID() << " : "
			<< " Makespan: " << Node->getSchedule()->makespan() << endl;

	minx = min(minx, Schedule->makespan());

	//reorder calendar of mutually exclusive events of E
	reorderCalendar(&E, &cal, Schedule);
	Node->saveCalendar(&cal);

	//If node is not in multi-schedule graph, add to MSG and meta-schedule
	//if (!msg.dejavu(Node)){
		//add node to multi-schedule graph
		msg.addNode(Node);

		//recursion
		metaSchedule(newModel, cal, Node);//}
	//}
}

void Fractal::createMSG(Fractal::Model *model, Fractal::Context *context) {
	cal = {}, msg = {};
	scheduleSequence_nr = 0;
	//Set boundaries of reconvergence horizon to numerical limits to compute the base schedule
	Fractal::Horizon horizon(numeric_limits<int>::min(), numeric_limits<int>::max());

	//initialize crash config needed by the GA
	Fractal::scheduler_config crashconfig = {a, b, c, d, e};
	model->horizonStart = 0;
	model->horizonEnd = 999;

	//auto t1 = high_resolution_clock::now();
	//invoke GA to obtain root schedule
	Fractal::Scheduler activeScheduler(model, &crashconfig, &horizon);
	activeScheduler.evolve();

	model->zu = 1;
	Fractal::Schedule *rootSchedule = new Fractal::Schedule(activeScheduler.best());
	model->zu = 0;
	//auto t2 = high_resolution_clock::now();
	// Getting number of milliseconds as an integer.
	//auto ms_int = duration_cast<milliseconds>(t2 - t1);
	//cout << "Time to compute Base Schedule: " << ms_int.count() << "ms\n";

	vector<Event> ne = {-1};
	vector <Fractal::Event> events;
	//Set the ID of the base schedule
	rootSchedule->set_uid(scheduleSequence_nr++);
	rootSchedule->setParent(-1);
	//Export base schedule to a json file.
	rootSchedule->export_json("results/", events);

	//create root node, add root schedule to root node
	Fractal::MsgNode *rootNode = new Fractal::MsgNode(rootSchedule->uid(),
			rootSchedule, ne);

/*	cout << "Root Node: " << rootNode->getID() << " : " << "Jobs: "
				<< rootNode->getSchedule()->jobs()[0].wcet_fullspeed << ", "
				<< rootNode->getSchedule()->jobs()[1].wcet_fullspeed << ", "
				<< rootNode->getSchedule()->jobs()[2].wcet_fullspeed << ", "
				<< "Makespan: " << rootNode->getSchedule()->makespan() << endl << endl;*/

	cout << "Root Node: " << rootNode->getID() << " : "
			<< " Makespan: " << rootNode->getSchedule()->makespan() << endl;

	//Establish calendar of events from CM
	createCalendar(rootSchedule, context);

	//Save calendar in the node. This will be needed to reduce state space explosion.
	rootNode->saveCalendar(&cal);

	//add root node to multi-schedule graph
	msg.addNode(rootNode);
/*	vector<Fractal::Lock> jk = model->getml();
	for (auto l : jk) {
		cout << l.resource << ":" << l.set << ":" << l.release << endl;
	}*/

	//invoke meta-scheduler
	metaSchedule(*model, cal, rootNode);

	//n_metaschedule (*model,samp_freq,rootNode, cal);

	cout << "size of MsG: " << msg.numOfNodes() << endl;
		//output file for visualization
	msg.exportMSG();
}
