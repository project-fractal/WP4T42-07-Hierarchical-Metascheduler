#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <vector>
#include "json_tools.hpp"
#include "schedule.hpp"
#include "Event.h"

using json = nlohmann::json;

namespace Fractal {
struct Crash {
	int node;

	Crash(int node) :
			node(node) {
	}

	nlohmann::json tojson() {
		nlohmann::json crash = nlohmann::json::object();
		crash["node"] = node;
		return crash;
	}
};

struct Slack {
	int jobID;
	int eventTime;
	int newExecutionTime;
	Event_type eventType;

	Slack(int job, int time) :
			jobID(job), eventTime(0), newExecutionTime(time), eventType(SLACK){}

	void setEventTime (Fractal::Schedule& schedule, Fractal::Slack& event);
};

class Context {
public:
	Context(json serializedCtx);

	void faults();

	int getSlacks();
	std::vector<Event> events;

	void tolocal(const int proc, const int nprocs);
private:
	void slacks();

	std::vector<Event> faults_;
	std::vector<Event> slacks_;
};
} // namespace Fractal
#endif
