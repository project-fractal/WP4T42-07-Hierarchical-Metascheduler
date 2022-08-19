#ifndef Fractal_SCHEDULE_HPP
#define Fractal_SCHEDULE_HPP

#include <vector>
#include <string>

#include "base.hpp"
#include "json_tools.hpp"
#include "utils/gnuplot-iostream.h"
#include "Event.h"

extern int mpi_tasks, mpi_rank;

namespace Fractal {
class Schedule {
public:
	Schedule(std::vector<Job> jobs, std::vector<Message> messages,std::vector<Node> nodes, std::vector<Link> links);

	std::vector<Job>& jobs();
	std::vector<Message>& messages();
	std::vector<Node>& nodes();
	std::vector<Link>& links();

	int makespan();
	void set_makespan(int val);
	float energy();
	void set_energy(float val);
	int order();
	void set_order(int val);
	bool is_valid();
	void set_valid(bool flag);
	void set_uid(int val);
	int uid();
	void set_crashednode(int node);
	int crashednode();
	Job& job(int id);
	bool is_viable_for_job(int id);
	void add_slack(int id);
	std::vector<int> slacks();
	void set_slacks(std::vector<int> s);
	void set_jpo(std::vector<int> jpo);
	bool is_poi(int id);
	void setParent (int parent) {parent_=parent;}
	int parent() {return parent_;}

	void saveGenome(int *g) {genome = g;}

	template<typename TypeWithID> TypeWithID&
	find(int id, std::vector<TypeWithID> &data);

	nlohmann::json tojson(vector<Event> event_);
	void export_svg(std::string path);
	void export_json(std::string path, vector<Event> events);
private:
	std::vector<Job> jobs_;
	std::vector<Message> messages_;
	std::vector<Node> nodes_;
	std::vector<Link> links_;

	int makespan_;
	float energy_;
	int order_;
	bool valid_;
	int uid_;
	int crashednode_;
	int parent_;

	int *genome;

	std::vector<int> slacks_;
	std::vector<int> jpo_;
};
} // namespace Fractal
#endif
