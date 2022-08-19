#include "schedule.hpp"

#include <boost/tuple/tuple.hpp>
#include <iomanip>
#include <string>
#include "model.hpp"

using namespace std;
using json = nlohmann::json;

Fractal::Schedule::Schedule(vector<Fractal::Job> jobs, vector<Fractal::Message> messages, vector<Fractal::Node> nodes, vector<Fractal::Link> links) :
	jobs_(jobs), messages_(messages), nodes_(nodes), links_(links){
		makespan_ = 0;
		energy_ = 0;
		valid_ = false;
		crashednode_ = -1;
		uid_ = 0;
	}

vector<Fractal::Job>&
Fractal::Schedule::jobs() {
	return jobs_;
}

vector<Fractal::Message>&
Fractal::Schedule::messages() {
	return messages_;
}

vector<Fractal::Node>&
Fractal::Schedule::nodes() {
	return nodes_;
}

vector<Fractal::Link>&
Fractal::Schedule::links() {
	return links_;
}

int Fractal::Schedule::makespan() {
	return makespan_;
}

void Fractal::Schedule::set_makespan(int val) {
	makespan_ = val;
}

float Fractal::Schedule::energy() {
	return energy_;
}

void Fractal::Schedule::set_energy(float val) {
	energy_ = val;
}

int Fractal::Schedule::order() {
	return order_;
}

void Fractal::Schedule::set_order(int val) {
	order_ = val;
}

bool Fractal::Schedule::is_valid() {
	return valid_;
}

void Fractal::Schedule::set_valid(bool flag) {
	valid_ = flag;
}

void Fractal::Schedule::set_uid(int val) {
	uid_ = val;
}

int Fractal::Schedule::uid() {
	return uid_;
}

void Fractal::Schedule::set_crashednode(int node) {
	crashednode_ = node;
}

int Fractal::Schedule::crashednode() {
	return crashednode_;
}

void Fractal::Schedule::add_slack(int id) {
	slacks_.push_back(id);
}
vector<int> Fractal::Schedule::slacks() {
	return slacks_;
}

void Fractal::Schedule::set_slacks(vector<int> s) {
	slacks_ = s;
}

void Fractal::Schedule::set_jpo(vector<int> jpo) {
	jpo_ = jpo;
}

/**
 * tojson() - json representation of the schedule
 *
 * Return: a json object
 */
json Fractal::Schedule::tojson(vector<Event> events_) {
	json solution = json::object();
	solution["makespan"] = makespan_;
	solution["valid"] = valid_;
	solution["energy"] = energy_;
	solution["uid"] = uid_;
	//solution["slacks"] = slacks_;
	//solution["JPO"] = jpo_;
	json context_array = json::array();
	for (auto& e : events_) {
		context_array.push_back(e.tojson());
	}

	solution["contexts"] = context_array;
	solution["parent_schedule"] = parent_;

	json jobarray = json::array();
	for (auto &j : jobs_)
		jobarray.push_back(j.tojson());

	json msgarray = json::array();
	for (auto &m : messages_)
		msgarray.push_back(m.tojson());

	solution["jobs"] = jobarray;
	solution["messages"] = msgarray;

	// creating a new json array: starray, a copy of the jobarray
	json starray = json::array();
	for (auto &j : jobs_)
		starray.push_back(j.tojson());

	/*
	 // prints all the start times and the corresponding id's before arranging
	 for (int i = 0; i < 10; i++){
	 std::cout << starray[i]["start_time"] << " " << starray[i]["id"] << std::endl;
	 }*/

	// ordering the start_time in ascending order and assigning the respective id's
	for (int i = 0; i < Fractal::Schedule::jobs().size(); i++) {
		for (int j = i + 1; j < Fractal::Schedule::jobs().size(); j++) {
			if (starray[j]["start_time"] < starray[i]["start_time"]) {
				int temp_st = starray[i]["start_time"];
				int temp_id = starray[i]["id"];
				starray[i]["start_time"] = starray[j]["start_time"];
				starray[i]["id"] = starray[j]["id"];
				starray[j]["start_time"] = temp_st;
				starray[j]["id"] = temp_id;
			}
		}
	}

	/*
	 // prints all the start times and the corresponding id's after arranging
	 for (int i = 0; i < 10; i++){
	 std::cout << starray[i]["start_time"] << " " << starray[i]["id"] << std::endl;
	 }*/

	// assigning order number according to the start time
	int ord[Fractal::Schedule::jobs().size()];
	for (int i = 0; i < Fractal::Schedule::jobs().size(); i++) {
		if (i == 0) {
			ord[i] = 1;
			continue;
		}
		if (starray[i]["start_time"] == starray[i - 1]["start_time"]) {
			ord[i] = ord[i - 1];
		} else{
			ord[i] = ord[i - 1] + 1;
		}
	}

	/*
	 // prints job order and job id
	 for (int i = 0; i < 10; i++){
	 std::cout << ord[i] << " " << starray[i]["id"] << std::endl;
	 }*/

	// copying starray id to new array id_vals
	int id_vals[Fractal::Schedule::jobs().size()];
	for (int i = 0; i < Fractal::Schedule::jobs().size(); i++) {
		id_vals[i] = starray[i]["id"];
	}

	// arranging id_vals in ascending order and assigning the corresponding ord[], ord[] is edited here
	for (int i = 0; i < Fractal::Schedule::jobs().size(); i++) {
		for (int j = i + 1; j < Fractal::Schedule::jobs().size(); j++) {
			if (id_vals[j] < id_vals[i]) {
				int temp_val = id_vals[i];
				int temp_ord = ord[i];
				id_vals[i] = id_vals[j];
				ord[i] = ord[j];
				id_vals[j] = temp_val;
				ord[j] = temp_ord;
			}
		}
	}

	// writing the job order and job id to json
	for (int i = 0; i < Fractal::Schedule::jobs().size(); i++) {
		solution["jobs"][i]["position"] = ord[i];
	}

	return solution;
}

/**
 * job() - the job matching id
 * @id: the jobs id
 *
 * If no job with that id exists the behaviour is undefined.
 *
 * Return: a reference to the job.
 */
Fractal::Job&
Fractal::Schedule::job(int id) {
	return find(id, jobs_);
}

/**
 * find - searches a datastructure for item with given id
 * @id: the elements id
 * @data: the structure
 *
 * Loops over the structure and returns the first item that matches
 * the given id. If no such element exists, something unexpected
 * happens.
 */
template<typename TypeWithID> TypeWithID&
Fractal::Schedule::find(int id, vector<TypeWithID> &data) {
	auto compare = [id](TypeWithID &v) {
		return v.id == id;
	};
	auto item = find_if(data.begin(), data.end(), compare);
	return *item;
}

/**
 * export_json() - save this schedule
 * @path: name of the target directory
 *
 * This function creates a json file for this schedule that will
 * contain all information about tasks and messages. The filename is
 * created from the schedules uid and (if existing) the faulty node.
 */
void Fractal::Schedule::export_json(string path, vector<Event> events) {
	string filename = path + "schedule-";
	if (crashednode_ != -1)
		filename += to_string(crashednode_);
	else
		filename += "-";
		filename += to_string(uid_);

	json schedule = tojson(events);

	ludwig::dump_json(filename + ".json", schedule);
}

/**
 * export_svg() - illustrates this schedule
 * @path: name of the target directory
 *
 * This function creates a picture (svg) that illustrates the
 * schedule. The picture will show allocations, timeslots and message
 * passing. It is stored at the given path. The filename is created
 * from the schedules uid and (if existing) the faulty node.
 */
void Fractal::Schedule::export_svg(string path) {
	string filename = path + "schedule-";
	if (crashednode_ != -1)
		filename += to_string(crashednode_);
	else
		filename += "base";
	filename += "-";
	filename += to_string(uid_);

	Gnuplot gp(">" + filename + ".gp");

	/* set output to png */
	gp << "set terminal svg\n" << "set output '" << filename << ".svg'\n";

	/* set the diagram title */
	gp << "set title 'Makespan: " << makespan_ << " Engergy: "
			<< setprecision(3) << energy_ << "'\n";

	/* set parameters */
	gp << "set yrange [-1:]\n" << "set xrange [-1:]\n" << "set xtics nomirror\n"
			<< "set ytics nomirror\n" << "set grid x y\n" << "unset key\n"
			<< "set border 3\n" << "set size 1, 1\n"
			<< "set linetype 3 lc rgb '#611543'\n"
			<< "set linetype 10 lc rgb '#ff9900'\n"
			<< "set style arrow 1 head size screen 0.02, 20 filled lt 3 lw 2\n"
			<< "set style arrow 2 head size screen 0.02, 20 filled lt 10 lw 2\n";

	/* Mommy's little helpers */
	gp << "lbl_offs(a,b) = a + 0.5*(b-a)\n";

	/* plotting */
	gp
			<< "plot '-' using ($2) : ($4) : (($3)-($2)) : (0.0) : yticlabel(sprintf('n_{%d}', ($4))) "
			<< "with vector as 1, "
			<< "'-' using (lbl_offs($2,$3)) : ($4) : (sprintf('j_{%d}', ($1))) "
			<< "with labels right offset 0,1,0, "
			<< "'-' using ($2) : ($4) : (($3)-($2)) : (($5)-($4)) with vector as 2, "
			<< "'-' using (lbl_offs($2,$3)) : (lbl_offs($4,$5)) : (sprintf('m_{%d}', ($1))) "
			<< "with labels center\n";

	/* collect data */
	map<int, int> alloc;

	// Jobs: id, start, end, node
	vector<boost::tuple<int, int, int, int>> job_data;
	for (auto &job : jobs_) {
		alloc[job.id] = job.runs_on;
		job_data.push_back(
				boost::make_tuple(job.id, job.start_time,
						job.start_time + job.wcet_at_freq, job.runs_on));
	}

	// Messages: id, start, end, src-node, dst-node
	vector<boost::tuple<int, int, int, int, int>> msg_data;
	for (auto &msg : messages_) {
		if (alloc[msg.sender] != alloc[msg.receiver]) {
			int nhops = msg.route.size();
			msg_data.push_back(
					boost::make_tuple(msg.id, msg.injection_time,
							msg.injection_time + nhops * msg.compressed_size,
							alloc[msg.sender], alloc[msg.receiver]));
		}
	}

	/* sending data */
	gp.send1d(job_data);		// for the jobs
	gp.send1d(job_data);		// for their labels
	gp.send1d(msg_data);		// for the messages
	gp.send1d(msg_data);		// for their labels

}

/**
 * is_viable_for_job() - checks if slack of given job will lead to a new state
 * @id: the jobs id
 *
 * Return: true, if the given job starts within the convergation
 * window.
 */
bool Fractal::Schedule::is_viable_for_job(int id) {
	if (uid_ == 0)
		return true;

	Job &candidate = job(id);

	Lock c_lock = { candidate.start_time, candidate.start_time
			+ candidate.wcet_at_freq, 1 };

	for (auto &j : jobs_) {
		if (j.id == candidate.id)
			continue;

		Lock j_lock = { j.start_time, j.start_time + j.wcet_at_freq, 1 };

		if (j_lock == c_lock)
			return true;

		const int j_finish = j.start_time + j.wcet_at_freq;
		const int c_start = candidate.start_time;

		if (j_finish > c_start)
			continue;

		if ((c_start - j_finish) < WINDOW_SZ) {
			const int bit = pow(2, j.id);
			if ((uid_ & bit) != 0)
				return true;
		}
	}

	return false;
}

/**
 * is_poi() - if a job has a direct successor
 * @id: the job
 */
bool Fractal::Schedule::is_poi(int id) {
	Job current = this->job(id);
	bool last_on_node = true;
	bool has_successor = false;

	for (auto &other : this->jobs()) {
		if (other.id == current.id)
			continue;

		if ((other.runs_on == current.runs_on)
				&& (other.start_time
						== current.start_time + current.wcet_at_freq)) {
			last_on_node = false;
			break;
		}
	}

	for (auto msg : this->messages()) {
		if (msg.sender != current.id)
			continue;

		int msg_finish = msg.injection_time
				+ msg.route.size() * msg.compressed_size;
		Job receiver = this->job(msg.receiver);
		if (receiver.start_time == msg_finish) {
			has_successor = true;
			break;
		}
	}

	return has_successor || !last_on_node;
}
