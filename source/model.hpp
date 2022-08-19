#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include <map>
#include <string>
#include <set>
#include <queue>
#include <fstream>
#include "GraphElements.h"
#include "Graph.h"
#include "YenTopKShortestPathsAlg.h"
#include <bits/stdc++.h>
#include "base.hpp"
#include "schedule.hpp"
#include "json_tools.hpp"
#include "utils/tsort.hpp"

using namespace std;

using json = nlohmann::json;

enum Resource_type {INVALID_RESOURCE, ROUTER, END_SYSTEM, GATEWAY, SWITCH};

namespace Fractal {
struct Lock {
	int set;			// first instant the lock is set
	int release;		// first instant the lock is not set
	int resource;		// the resource to lock

	/* Check if two locks are overlapping */
	bool operator==(const Lock &lk) {
		if (resource == lk.resource) {
			return (((set <= lk.set) && (lk.set < release)) || ((lk.set <= set) && (set < lk.release)));
		}
		else {
			return false;
		}
	}
};

struct Horizon {
	int start;
	int end;

	Horizon(int begin, int reconverge) : start(begin), end(reconverge){};
};

struct msgCost {
	int id;
	int cost;
};

class Model {
public:
	Model(json serializedModel);

	int njobs();
	int nmsgs();
	int nnodes();
	int nschemes();
	int max_routes();
	int norders();
	int nfrequencies();

	const vector<Message>& msgs();
	const vector<Node>& nodes();
	const vector<Link>& links();
	const vector<Job>& jobs();
	const vector<Scheme>& schemes();

	Job& job(int id);
	Node& node(int id);
	Message& message(int id);

	bool is_router(int id);
	bool link_exists(int start, int end);
	Route route(int from, int to, int number);
	Schedule schedule(int *genes, vector<Job> pinnedj, vector<Message> pinnedm);

	void set_lock (vector<Lock>& locks, Job j);
	int getMsg_cost (vector <int> constraints);
	int get_schedule (int instant, int duration, int resource,
			vector<Lock> &locks);

	int find_slot(int instant, int duration, vector<int> resources,
			vector<Lock> &locks);

	void lock_slot(int slot, int slot_sz, vector<int> resources,
			vector<Lock> &locks);

	int find_and_lock(int earliest, int slot_sz, vector<int> res,
			vector<Lock> &locks);

	vector<Route> find_routes(int from, int to);
	Route find_rutes(int from, int to);
	Route find_kShortestRoute(int from, int to, int index);
	vector<vector<Message>::const_iterator> sent_by(int job);
	vector<vector<Message>::const_iterator> received_by(int job);

	void initialize();
	void remove_node(int id);
	void remove_link(int start, int end);
	void shorten_job(int id);
	
	int horizonStart;
	int horizonEnd;

	void sort_js();
	void unsort_js();
	void apply_events(vector <Fractal::Event> events);

	vector<Lock> getml() {return msgLocks;}

	int zu=0;

private:
	template<typename TypeWithID>
	typename vector<TypeWithID>::iterator find(int id,
			vector<TypeWithID> &data);

	void build_messages(json messages);
	void build_nodes(json nodes);
	void build_jobs(json jobs);
	void build_links(json links);
	void build_schemes(json schemes);
	//void orders_init();
	//void routes_init();
	void orders();
	void kShortestRoutes();

	bool BFS(int source, int destination, int pred[]);
	void testYenAlg( const int& k,
			linkl* lk,
			const int& size,
			const int& s,
			const int& d,
			const int& nodes );

	//Message& message(int id);
	Scheme& scheme(int id);

	int nnodes_;	    // number of nodes in the network
	int njobs_;		    // number of jobs in the DAG
	int nmsgs_;		    // number of edges (messages) in the DAG
	int nlinks_;	    // number of edges (links) in the network
	int nschemes_;	    // number of different compression schemes
	int nfrequencies_;

	vector<Message> msgs_;	    // holds messages
	vector<Job> jobs_;	    // holds jobs
	vector<Node> nodes_;	    // holds nodes
	vector<Link> links_;	    // holds links
	vector<Scheme> schemes_;   // holds compression schemes
	vector<Order> orders_;    // holds topol. orderings of messages
	vector<int> frequencies_;

	map<int, vector<Route>> rutes_;
	map<int, vector<Route>> routes_;	// node -> all routes starting there
	map<int, map<int, vector<vector<int>>>> KSP;

	map<int, vector<Lock>> jobLocks = {};
	vector<Lock> msgLocks = {};//Map of job allocations
	int findandlockJob (int earliest, int slotsz, int resource);
	int find_jobslot(int instant, int duration, int resource);
	int findandlockMsg (int earliest, int slotsz, Route route, int id);
	int find_msgslot(int instant, int duration, Route route);

	bool notValid(Route route);
	bool allocateMsgs (Job* j, vector<Message> pinnedm, vector<Job> pinnedj);
	int msgSize = 32;

	vector <Fractal::Lock> getlocks(int instant, int duration, Route route);
};

// Class to represent a graph
class Graph {
	// No. of vertices'
	int V;
	// Pointer to an array containing adjacency listsList
	//list<int> *adj;

	// vector containing adjacency lists
	vector<list<int>> adj;
public:
	// Constructor
	Graph(int V);

	// Function to add an edge to graph
	void addEdge(int u, int v);
	// prints a Topological Sort of the complete graph
	vector<int> topologicalSort(vector<Job>* order);
};
} // namespace Fractal
#endif
