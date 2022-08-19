#include "context.hpp"

#include <numeric>

using namespace std;
using json = nlohmann::json;

/**
 * Context() - constructs a new context
 * @serializedCtx: a json object describing a valid context
 *
 * The json object must contain two arrays called "crashes" and
 * "slacks". Objects inside these arrays must at least contain the
 * fields also mentioned in the type definition of Fractal::Crash and
 * Fractal::Slack.
 */
Fractal::Context::Context(json serializedCtx) {
	//Get event in context model and update...

	//case "slacks":
	for (auto &serializedEvent : serializedCtx["slacks"]) {
		Fractal::Event slack(serializedEvent["job"].get<int>());
		slack.value = serializedEvent["NewExecutionTime"];
		slack.eventType = SLACK;
		//add slack event to context events
		events.push_back(slack);
	}

	//case "faults":
	for (auto &serializedEvent : serializedCtx["faults"]) {
		if (serializedEvent["NodeId"].get<int>() == -1){
			return;
		}

		Fractal::Event fault(serializedEvent["NodeId"].get<int>());
		fault.eventType = FAULT;
		fault.value = serializedEvent["Occur"];
		string subType = serializedEvent["type"];

		if (subType == "crash") {
			fault.subCategory = CRASH;
		}

		//add fault event to context events
		events.push_back(fault);
	}
}

/**
 * crashes() - all crash events mentioned in the context
 *
 * Return: all crash events
 */
/*vector<Fractal::Crash> Fractal::Context::crashes() {
 return crashes_;
 }*/

void Fractal::Context::slacks() {
	for (Fractal::Event S : events) {
		if (S.eventType == SLACK) {
			slacks_.push_back(S);
		}
	}
}

void Fractal::Context::faults() {
	for (Fractal::Event F : events) {
		if (F.eventType == FAULT) {
			faults_.push_back(F);
		}
	}
}

void setEventTime(Fractal::Schedule &schedule, Fractal::Slack &event) {
	vector<Fractal::Job> j = schedule.jobs();
	for (Fractal::Job &f : j) {
		if (f.id == event.jobID) {
			event.eventTime = f.start_time + event.newExecutionTime;
		}
	}
}

int Fractal::Context::getSlacks() {
	slacks();
	return slacks_.size();
}
/**
 * slacks() - all slack events mentioned in the context
 *
 * Return: all slack events
 */
/*vector<Fractal::Slack>&
 Fractal::Context::slacks() {
 return slacks_;
 }*/

/**
 * tolocal() - shrinks to context to events needed by current process
 * @proc: id of current process
 * @nprocs: amount of processes
 */
/*void Fractal::Context::tolocal(const int proc, const int nprocs) {
 const int kNevents = crashes_.size();
 const int kStart = kNevents / nprocs * proc
 + max(proc - (nprocs - kNevents % nprocs), 0);
 const int kSize = (kNevents + proc) / nprocs;

 crashes_ = vector<Crash>(crashes_.begin() + kStart,
 crashes_.begin() + kStart + kSize);
 }*/
