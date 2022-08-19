#include "model.hpp"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <climits>
#include <stdexcept>

using namespace std;
using json = nlohmann::json;

Fractal::Model::Model(json serializedModel) {
	json lm = serializedModel["application"];
	json pm = serializedModel["platform"];

	build_messages(lm["messages"]);
	build_nodes(pm["nodes"]);
	build_jobs(lm["jobs"]);
	build_links(pm["links"]);
	build_schemes(pm["schemes"]);

	frequencies_ = pm["frequencies"].get<vector<int>>();
	nfrequencies_ = frequencies_.size();

	/* sort the frequencies so that the highest frequency is always the
	 last */
	std::sort(frequencies_.begin(), frequencies_.end());

	nnodes_ = nodes_.size();
	njobs_ = jobs_.size();
	nmsgs_ = msgs_.size();
	nlinks_ = links_.size();
	nschemes_ = schemes_.size();

	//orders_init();
	//initialize();
	//orders();
	kShortestRoutes();
}

void Fractal::Model::apply_events(vector <Fractal::Event> events) {
	for (auto& event : events) {
		switch (event.eventType) {
			case SLACK:
				event.old = job(event.target).wcet_fullspeed;
				job(event.target).wcet_fullspeed = event.value;
				break;
		}
	}
}

bool Fractal::Model::BFS(int source, int destination, int pred[]) {
	// a queue to maintain queue of vertices whose
	// adjacency list is to be scanned as per normal
	// DFS algorithm
	list<int> queue;
	vector<int> adj[nnodes_];

	for (auto& edge : links_){
		adj[edge.start].push_back(edge.end);
		adj[edge.end].push_back(edge.start);
	}

	// boolean array visited[] which stores the
	// information whether ith vertex is reached
	// at least once in the Breadth first search
	bool visited[nnodes_];

	// initially all vertices are unvisited
	// so v[i] for all i is false
	// and as no path is yet constructed
	// dist[i] for all i set to infinity
	for (int i = 0; i < nnodes_; i++) {
		visited[i] = false;
		pred[i] = -1;
	}

	// now source is first to be visited and
	// distance from source to itself should be 0
	visited[source] = true;
	queue.push_back(source);

	// standard BFS algorithm
	while (!queue.empty()) {
		int u = queue.front();
		queue.pop_front();
		for (unsigned int i = 0; i < adj[u].size(); i++) {
			if (visited[adj[u][i]] == false) {
				visited[adj[u][i]] = true;
				pred[adj[u][i]] = u;
				queue.push_back(adj[u][i]);

				// We stop BFS when we find
				// destination.
				if (adj[u][i] == destination)
					return true;
			}
		}
	}

	return false;
}

bool Fractal::Model::notValid(Route route) {
	route.pop_back();
	route.erase(route.begin());
	for (auto n : route) {
		if (!node(n).is_router) {
			return true;
		}
	}
	return false;
}

void Fractal::Model::testYenAlg( const int& k,
		linkl* lk,
		const int& size,
		const int& s,
		const int& d,
		const int& nodes )
{
	Graphk my_graph;

	my_graph.set_number_vertices( nodes );

	for ( int i = 0; i < size; i++ )
	{
		my_graph.add_link( lk[ i ].u, lk[ i ].v, lk[ i ].weight );
	}

	my_graph.setv();

	YenTopKShortestPathsAlg yenAlg(my_graph,
			my_graph.get_vertex(s),
			my_graph.get_vertex(d) );

	// Output the k-shortest paths
	int i = 0;
	vector <int> route;

	while( yenAlg.has_next() && i < k )
	{
		route = yenAlg.next()->route();
		if (notValid(route)) {
			++i;
			continue;
		}
		else {
			KSP[route.front()][route.back()].push_back(route);
			++i;
		}
	}
}

void Fractal::Model::sort_js(){
	sort (jobs_.begin(), jobs_.end(), [](Fractal::Job& lhs, Fractal::Job&rhs) {
		return lhs.wcet_fullspeed<rhs.wcet_fullspeed;
	});
}

void Fractal::Model::unsort_js(){
	sort (jobs_.begin(), jobs_.end(), [](Fractal::Job& lhs, Fractal::Job&rhs) {
		return lhs.id<rhs.id;
	});
}

void Fractal::Model::kShortestRoutes(){
	linkl lks[(nlinks_*2)];
	int k = 5;
	int nodes = nnodes_;
	int counter = 0;
	int lks_size = sizeof(lks)/sizeof(lks[0]);

	for (auto& edge : links_){
		lks[counter] = {edge.start, edge.end};
		counter++;
		lks[counter] = {edge.end, edge.start};
		counter++;
	}

	vector<int> sources;
	for (auto& e : nodes_) {
		if (e.is_router == false) {
			sources.push_back(e.id);
		}
	}

	for (int& i : sources) {
		vector <int> destinations;
		copy_if(sources.begin(), sources.end(), back_inserter(destinations), [i] (const int& f) {return f != i;});

		for (int& h: destinations) {
			int start = i;
			int end = h;

			testYenAlg(k, lks, lks_size, start, end, nodes);
		}
	}

	// For accessing outer map
	map<int, map<int, vector<vector<int>>>>::iterator itr;
	// For accessing inner map
	map<int, vector<vector<int>>>::iterator ptr;

	//sort paths from shortest to longest.
	for (itr = KSP.begin(); itr != KSP.end(); itr++) {
		for (ptr = itr->second.begin(); ptr != itr->second.end(); ptr++) {
			sort (ptr->second.begin(), ptr->second.end(), [](const vector<int>&a, const vector<int>&b){return a.size() < b.size();});
		}
	}
}

void Fractal::Model::orders() {
	int pred[nnodes_];

	vector<int> sources;
	for (auto& e : nodes_) {
		if (e.is_router == false) {
			sources.push_back(e.id);
		}
	}

	for (int i : sources) {
		vector <int> destinations;
		copy_if(sources.begin(), sources.end(), back_inserter(destinations), [i] (const int& f) {return f != i;});

		for (int h: destinations) {
			BFS(h, i, pred);
			vector<int> path;
			int crawl = i;
			path.push_back(crawl);
			while (pred[crawl] != -1) {
				path.push_back(pred[crawl]);
				crawl = pred[crawl];}

			rutes_[i].push_back(path);
		}
	}
}

Route Fractal::Model::find_rutes(int from, int to) {
	vector < Route > &all = rutes_[from];
	Route to_dst;

	for (Route p : all)
		if (p.back() == to){
			p.erase(p.begin());
			return p;
		}

	Route n{};
	return n;
}

/**
 * initialize() - initializes routing and topology
 *
 * Use this after modifying or cloning the model.
 */
/*void Fractal::Model::initialize() {
	routes_.clear();
	routes_init();
}*/

/**
 * shorten_job() - decreases the worst-case execution time
 * @id: the jobs id
 *
 * Replaces wcet_fullspeed with mcet.
 */
void Fractal::Model::shorten_job(int id) {
	Fractal::Job &j = job(id);
	j.wcet_fullspeed = j.mcet;
}

/**
 * remove_node() - remove a node from the model
 * @id: the id of the node
 *
 * Removes the node with the given id from nodes_ and also all links
 * from links_ that start or end at this node. It removes the node
 * also from the job's partition.
 */
void Fractal::Model::remove_node(int id) {
	auto node = find_if(nodes_.begin(), nodes_.end(), [id](Node &node) {
		return node.id == id;
	});

	if (node != nodes_.end()) {
		nodes_.erase(node);
		nnodes_--;
	}

	for (auto link = links_.begin(); link != links_.end();) {
		if (link->start == id || link->end == id) {
			link = links_.erase(link);
			nlinks_--;
		} else
			++link;
	}

	for (auto &job : jobs_) {
		auto pos = std::find(job.can_run_on.begin(), job.can_run_on.end(), id);
		if (pos != job.can_run_on.end())
			job.can_run_on.erase(pos);
	}
}

/**
 * remove_link() - removes a link from the model
 * @start: the start node
 * @end: the end node
 *
 * Removes the link between start and end.
 */
void Fractal::Model::remove_link(int start, int end) {
	int lo = min(start, end);
	int hi = max(start, end);
	auto link = find_if(links_.begin(), links_.end(),
			[lo, hi](const Link &link) {
				return link.start == lo && link.end == hi;
			});
	if (link != links_.end())
		links_.erase(link);
}

/**
 * build_messages() - builds messages from json
 * @messages: json representing an array of messages
 *
 * Loops over the array and creates a Fractal::Message for each
 * item. The message is then pushed to the msgs_ container.
 */
void Fractal::Model::build_messages(json messages) {
	for (auto &parsedMsg : messages) {
		Fractal::Message m(parsedMsg["id"], parsedMsg["size"],
				parsedMsg["sender"], parsedMsg["receiver"],
				parsedMsg["timetriggered"]);
		m.injection_time = 0;
		m.arrival_time = 0;
		m.route = {};
		msgs_.push_back(m);
	}
}

/**
 * build_nodes() - builds nodes from json
 * @nodes: json representing an array of nodes
 *
 * Loops over the array and creates a Fractal::Node for each
 * item. The node is then pushed to the nodes_ container.
 */
void Fractal::Model::build_nodes(json nodes) {
	for (auto &parsedNode : nodes) {
		json temperature = parsedNode["temperature"];
		Fractal::Node n(parsedNode["id"], parsedNode["is_router"],
				temperature.is_null() ? INT_MAX : temperature.get<int>());
		nodes_.push_back(n);

		if (n.is_router == false) {
			jobLocks[n.id] = {};
		}
	}
}

/**
 * build_jobs() - builds jobs from json
 * @jobs: json representing an array of jobs
 *
 * Loops over the array and creates a Fractal::Job for each
 * item. The job is then pushed to the jobs_ container.
 */
void Fractal::Model::build_jobs(json jobs) {
	vector<int> computeNodeIDs;
	int computeNodes = std::count_if(nodes_.begin(), nodes_.end(),
			[](Fractal::Node n) {
				return n.is_router == false;
			});

	/*std::transform (nodes_.begin (),
	 nodes_.begin () + computeNodes,
	 std::back_inserter (computeNodeIDs),
	 [](Fractal::Node n){
	 return n.id;
	 });*/
	for (auto i : nodes_)
		if (!i.is_router)
			computeNodeIDs.push_back(i.id);

	for (auto parsedJob : jobs) {
		json temperature = parsedJob["max_temperature"];
		json can_run_on = parsedJob["can_run_on"];

		Fractal::Job j(parsedJob["id"], parsedJob["wcet_fullspeed"],
				parsedJob["mcet"], parsedJob["deadline"],
				temperature.is_null() ? INT_MAX : temperature.get<int>(),
				(can_run_on.is_null() || can_run_on.empty()) ?
						computeNodeIDs : can_run_on.get<vector<int>>());
		j.start_time = 0;
		jobs_.push_back(j);
	}
}

/**
 * build_links() - builds links from json
 * @jobs: json representing an array of links
 *
 * Loops over the array and creates a Fractal::Link for each item. The
 * link is then pushed to the links_ container. The container is then
 * sorted by ascending start-nodes and ascending end-nodes.
 */
void Fractal::Model::build_links(json links) {
	int counter = 0;
	for (auto parsedLink : links) {
		Fractal::Link l(counter++, parsedLink["start"], parsedLink["end"], parsedLink["delay"]);
		links_.push_back(l);
	}

	auto compare = [](const Fractal::Link &lhs, const Fractal::Link &rhs) {
		return (lhs.start == rhs.start) ?
				lhs.end < rhs.end : lhs.start < rhs.start;
	};

	sort(links_.begin(), links_.end(), compare);
}

/**
 * schemes_init() - builds structs from json-formatted schemes
 */
void Fractal::Model::build_schemes(json schemes) {
	for (auto parsedScheme : schemes) {
		Fractal::Scheme s(parsedScheme["id"], parsedScheme["wcdt"],
				parsedScheme["wcct"], parsedScheme["wccr"]);
		schemes_.push_back(s);
	}
}

/**
 * orders_init() - find all topological orders of messages
 *
 * Initializes the orderings_ vector with all topological orderings of
 * the messages to that the precedence relations stay intact.
 */
/*void Fractal::Model::orders_init() {
	ludwig::tsort t = ludwig::tsort(nmsgs_);

	for (auto cur : msgs_) {
		int recv = cur.receiver;
		for (auto succ : sent_by(recv))
			t.add_edge(cur.id, succ->id);
	}

	t.sort();

	orders_ = t.get_orders();
}*/

/**
 * njobs() - the number of jobs in the DAG
 */
int Fractal::Model::njobs() {
	return njobs_;
}

/**
 * nmsgs() - the number of edges in the DAG
 */
int Fractal::Model::nmsgs() {
	return nmsgs_;
}

/**
 * nnodes() - the number of nodes in the network
 */
int Fractal::Model::nnodes() {
	return nnodes_;
}

/**
 * max_routes () - the maximum number of routes between two nodes
 */
int Fractal::Model::max_routes() {
	int tmp = 0;
	for (auto from : nodes_)
		for (auto to : nodes_) {
			if (to.is_router || from.id == to.id)
				continue;

			int nroutes = find_routes(from.id, to.id).size();
			tmp = max(tmp, nroutes);
		}
	return tmp;
}

/**
 * msgs() - set of edges in the DAG
 *
 * Return: a constant reference to the message container
 */
const vector<Fractal::Message>&
Fractal::Model::msgs() {
	return msgs_;
}

/**
 * nodes() - set of nodes in the network
 *
 * Return: a constant reference to the nodes container
 */
const vector<Fractal::Node>&
Fractal::Model::nodes() {
	return nodes_;
}

/**
 * jobs() - set of jobs
 *
 * Return: a constant reference to the jobs container
 */
const vector<Fractal::Job>&
Fractal::Model::jobs() {
	return jobs_;
}

/**
 * links() - set of edges in the network graph
 *
 * Return: a constant reference to the links container
 */
const vector<Fractal::Link>&
Fractal::Model::links() {
	return links_;
}

/**
 * schemes() - set of compression schemes
 *
 * Return: a constant reference to the schemes container
 */
const vector<Fractal::Scheme>&
Fractal::Model::schemes() {
	return schemes_;
}

/**
 * norders() - number of diff. topol. orders
 */
int Fractal::Model::norders() {
	return orders_.size();
}

int Fractal::Model::nfrequencies() {
	return nfrequencies_;
}

/**
 * is_router() - checks if a node is a router
 * @id: the id of the node to check
 *
 * Returns true, iff the node matching the given id is a router.
 */
bool Fractal::Model::is_router(int id) {
	Fractal::Node n = node(id);
	return n.is_router;
}

/**
 * link_exists() - checks if a link exists
 * @start: a node-id
 * @end: another node-id
 *
 * Returns true, iff there is a direct connection between start and
 * end in the network graph.
 */
bool Fractal::Model::link_exists(int start, int end) {
	auto compare = [start, end](Fractal::Link &v) {
		return ((v.start == start && v.end == end)
				|| (v.start == end && v.end == start));
	};
	auto link = find_if(links_.begin(), links_.end(), compare);
	return link != links_.end();
}

/**
 * message() - get a message
 * @id: the id of the message you want
 *
 * Return: reference to the message
 */
Fractal::Message&
Fractal::Model::message(int id) {
	auto it = find(id, msgs_);
	if (it == msgs_.end())
		throw runtime_error("No such message.");
	return *it;
}

/**
 * scheme() - get a compression scheme
 * @id: the id of the cs you want
 *
 * Return: reference to the scheme
 */
Fractal::Scheme&
Fractal::Model::scheme(int id) {
	auto it = find(id, schemes_);
	if (it == schemes_.end())
		throw runtime_error("No such scheme.");
	return *it;
}

/**
 * nschemes() - the number of available compression schemes
 */
int Fractal::Model::nschemes() {
	return nschemes_;
}

/**
 * node() - get a node
 * @id: the id of the node you want
 *
 * Return: reference to the node
 */
Fractal::Node&
Fractal::Model::node(int id) {
	auto it = find(id, nodes_);
	if (it == nodes_.end())
		throw runtime_error("No such node.");
	return *it;
}

/**
 * job() - get a job
 * @id: the id of the job you want
 *
 * Return: reference to the job
 */
Fractal::Job&
Fractal::Model::job(int id) {
	auto it = find(id, jobs_);
	if (it == jobs_.end())
		throw runtime_error("No such job.");
	return *it;
}

/**
 * sent_by() - messages sent by a specific job
 * @id: the jobs id
 * Return: vector of constant iterators
 */
vector<vector<Fractal::Message>::const_iterator> Fractal::Model::sent_by(
		int job) {
	vector < vector < Fractal::Message > ::const_iterator > matches;
	for (auto m = msgs_.begin(); m != msgs_.end(); ++m)
		if (m->sender == job)
			matches.push_back(m);

	return matches;
}

/**
 * received_by() - messages received by a specific job
 * @id: the jobs id
 * Return: vector of constant iterators
 */
vector<vector<Fractal::Message>::const_iterator> Fractal::Model::received_by(
		int job) {
	vector < vector < Fractal::Message > ::const_iterator > matches;
	for (auto m = msgs_.begin(); m != msgs_.end(); ++m)
		if (m->receiver == job)
			matches.push_back(m);

	return matches;
}

/**
 * find - searches a datastructure for item with given id
 * @id: the elements id
 * @data: the structure
 *
 * Loops over the structure and returns an iterator to the first item
 * that matches the given id. If no such element exists, data.end() is
 * returned.
 */
template<typename TypeWithID>
typename vector<TypeWithID>::iterator Fractal::Model::find(int id,
		vector<TypeWithID> &data) {
	auto compare = [id](TypeWithID &v) {
		return v.id == id;
	};
	return find_if(data.begin(), data.end(), compare);
}

/**
 * find_routes() - a list of possible routes
 * @from: the source
 * @to: the destination
 *
 * Return: a vector containing routes (vector)
 */
vector<Route> Fractal::Model::find_routes(int from, int to) {
	vector < Route > &all = routes_[from];
	vector < Route > to_dst;
	for (auto p : all)
		if (p.back() == to)
			to_dst.push_back(p);
	return to_dst;
}

/**
 * route() - a specific route
 * @from: the source
 * @to: the destination
 * @number: the number of the route (if there are multiple)
 *
 * Return: a vector of ints (the nodes) or empty vector, iff no route
 * exists
 */
Route Fractal::Model::route(int from, int to, int number) {
	vector < Route > p = find_routes(from, to);
	if (p.empty())
		return Route();

	return p.at(number % p.size());
}

/**
 * find_slot() - find a slot to use a resource
 * @instant: the earliest start time of your slot
 * @duration: the size of the slot
 * @resources: the resources
 *
 * Finds a slot for exclusively accessing a list of shared resources.
 *
 * Return: The earliest instant which guarantees exclusive access.
 */
int Fractal::Model::find_slot(int instant, int duration, vector<int> resources,
		vector<Lock> &locks) {
	Fractal::Lock lck = { instant, instant + duration, resources.front() };

	resources.erase(resources.begin());
	/* Anchor */
	if (resources.empty())
		return instant;

	while (true) {
		/* If an overlapping lock exists, choose a new slot */
		if (std::find(locks.begin(), locks.end(), lck) != locks.end()) {
			lck.set++;
			lck.release++;
			continue;
		}

		/* If a slot for the current resource is found, check if it also
		 works with the other resources. */
		float slot = find_slot(lck.release, duration, resources, locks);
		if (slot == lck.release) {
			/* Yes, a slot is found */
			break;
		} else {
			/* No, choose another slot */
			lck.set = slot;
			lck.release = lck.set + duration;
		}
	}

	/* Finally, return the slot */
	return lck.set;
}

/**
 * lock_slot() - locks a slot for a list of resources
 * @slot: the instant the slot starts
 * @slot_sz: the slot size
 * @resources: the resources you want to lock
 *
 * Inserts a lock of length slot_sz for each resource into locks
 * starting at slot.
 */
void Fractal::Model::lock_slot(int slot, int slot_sz, vector<int> resources,
		vector<Lock> &locks) {
	if (!resources.empty())
		resources.pop_back();
	int lck_set = slot;
	for (auto resource : resources) {
		Fractal::Lock lck = { lck_set, lck_set + slot_sz, resource };
		locks.push_back(lck);
		lck_set += slot_sz;
	}
}


/**
 * find_and_lock() - find and lock a slot
 * earliest: the earliest instant the slot should start
 * slot_sz: the slot size
 * resources: the resources that you want to access (links, nodes)
 * locks: container that holds locks
 *
 * Return: the start time of the slot. If no resources are given,
 * earliest is returned.
 */
int Fractal::Model::find_and_lock(int earliest, int slot_sz,
		vector<int> resources, vector<Lock> &locks) {
	if (resources.empty())
		return earliest;

	int slot = find_slot(earliest, slot_sz, resources, locks);
	lock_slot(slot, slot_sz, resources, locks);

	return slot;
}

int Fractal::Model::find_jobslot(int instant, int duration, int resource) {
	Fractal::Lock lck = {instant, instant + duration, resource};
	/* Anchor */
	if (jobLocks[resource].empty()) {
		return instant;
	}

	while (true) {
	/* If an overlapping lock exists, choose a new slot */
		if (std::find(jobLocks[resource].begin(), jobLocks[resource].end(), lck) != jobLocks[resource].end()) {
			lck.set++;
			lck.release++;
			continue;
		}
		else {
			break;
		}
	}
	return lck.set;
}

vector <Fractal::Lock> Fractal::Model::getlocks(int instant, int duration, Route route) {
	vector <Fractal::Lock> msglocks;
	for (auto n : route) {
		int delay = find(n,links_)->delay;
		Fractal::Lock lck = {instant, (instant + (duration*delay)), n};
		msglocks.push_back(lck);
		instant = (instant + duration)*delay;
	}
	return msglocks;
}

int Fractal::Model::find_msgslot (int instant, int duration, Route route) {
	if (msgLocks.empty()) {
		return instant;
	}

	while (true) {
		int flag = 0;
		vector <Lock> msglocks = getlocks(instant, duration, route);
		for (auto lk : msglocks) {
			for (auto olk : msgLocks) {
				if ((olk.resource == lk.resource && olk.set<=lk.set && lk.set<olk.release) || (olk.resource == lk.resource && lk.set<=olk.set && olk.set<lk.release)) {
					flag = 1;
					//instant=olk.release;
					instant++;
					break;
				}
			}
			if (flag == 1) {
				break;
			}
		}
		if (flag == 0) {
			break;
		}
	}
	return instant;
}

Fractal::Graph::Graph(int V): adj(V) {
	this->V = V;
	//adj = new list<int> [V];
}

void Fractal::Graph::addEdge(int u, int v) {
	//adj[u].push_back(v);
	if (adj.size() <= u) {
		adj.resize(u);
	}
	adj.at(u).push_back(v);
}

// The function to do Topological Sort.
vector<int> Fractal::Graph::topologicalSort(vector<Job>* order) {
	// Create a vector to store indegrees of all vertices. Initialize all indegrees as 0.
	vector<int> in_degree(V, 0);

	// Traverse adjacency lists to fill indegrees of vertices. This step takes O(V+E) time
	for (int u = 0; u < V; u++) {
		list<int>::iterator itr;
		for (itr = adj.at(u).begin(); itr != adj.at(u).end(); itr++)
			in_degree[*itr]++;
	}

	//Temporary vector to hold adjacent vertices
	vector<Job> tmp;

	// Create a queue and enqueue all vertices with indegree 0
	queue<int> q;
	for (int i = 0; i < V; i++)
		if (in_degree[i] == 0)
			tmp.push_back(order->at(i));

	sort(tmp.begin(), tmp.end(), [](const Job& job1, const Job& job2){
		return job1.weight > job2.weight;
	});

	q.push(tmp.front().id);

	// Initialize count of visited vertices
	int cnt = 0;

	// Create a vector to store result (A topological ordering of the vertices)
	vector<int> top_order;

	// One by one dequeue vertices from queue and enqueue adjacents if indegree of adjacent becomes 0
	while (!tmp.empty()) {
		// Extract front of queue (or perform dequeue) and add it to topological order
		int u = q.front();
		q.pop();
		top_order.push_back(u);

		// Iterate through all neighbouring nodes of dequeued node u and decrease their in-degree by 1
		list<int>::iterator itr;
		tmp.erase(tmp.begin());
		for (itr = adj.at(u).begin(); itr != adj.at(u).end(); itr++){
			// If in-degree becomes zero, add it to queue
			if (--in_degree[*itr] == 0)
				tmp.push_back(order->at(*itr));
		}
		sort(tmp.begin(), tmp.end(), [](const Job& job1, const Job& job2){
				return job1.weight > job2.weight;
			});

		q.push(tmp.front().id);
	}
	return top_order;
}

int Fractal::Model::findandlockJob (int earliest, int slotsz, int resource) {
	Fractal::Lock lck = {earliest, earliest+slotsz, resource};

	if (jobLocks[resource].empty()) {
		jobLocks[resource].push_back(lck);
		return earliest;
	}

	int slot = find_jobslot(earliest, slotsz, resource);
	lck = {slot, slot+slotsz, resource};
	jobLocks[resource].push_back(lck);

	return slot;
}

int Fractal::Model::findandlockMsg (int earliest, int slotsz, Route route, int id) {
	if (route.empty()) {
		message(id).arrival_time = earliest;
		return earliest;
	}

	vector<int> lks;
	for (int i=0;i<route.size()-1;i++) {
		int st = route[i];
		int ed = route[i+1];
		for (auto l:links_) {
			if ((l.start == st && l.end == ed) || (l.start == ed && l.end == st)) {
				lks.push_back(l.id);
			}
		}
	}

	int slot = find_msgslot(earliest, slotsz, lks);
	int injection = slot;

	for (auto n : lks) {
		int delay = find(n,links_)->delay;
		Fractal::Lock lck = {slot, (slot + (slotsz*delay)), n};
		msgLocks.push_back(lck);
		slot = slot + (slotsz*delay);
	}
	message(id).arrival_time = slot;

	return injection;
}

int earliestTime (vector <Fractal::Message> msgs_, int jobID) {
	int constraint = 0;
	int msgSize = 32;

	for (auto msg : msgs_) {
		if (msg.receiver == jobID) {
			//int hop_time = ceil((float)msg.size/(float)msgSize);
			//int hop_time = msg.size;
/*			int arrival = 0;
			if (msg.route.size() == 0) {
				arrival = msg.injection_time;
			} else {
				arrival = msg.injection_time + ((msg.route.size()-1) * hop_time);
			}
			constraint = max(constraint,arrival);*/
			constraint = max(constraint,msg.arrival_time);
		}
	}
	return constraint;
}

/**
 * schedule() - decodes a genome into a schedule
 * @genes: array containing the different genes
 * @pinned: the jobs located behind the slackjob that should be pinned
 *
 * Return: a schedule
 */

bool is_pinned(vector<Fractal::Message> pinnedm, int id){
	for (auto m : pinnedm) {
		if (m.id == id) {
			return true;
		}
	}
	return false;
}

bool Fractal::Model::allocateMsgs (Job* j, vector<Message> pinnedm, vector<Job> pinnedj) {
	vector<Message> cm;
	for (auto& msg : msgs_) {
		if (msg.sender == j->id) {
			cm.push_back(msg);
		}
	}

	sort(cm.begin(), cm.end(), [](const Message& m1, const Message& m2){
		return m1.weight > m2.weight;
	});

	for (auto& msg : cm) {
		auto pm = find(msg.id, pinnedm);
		if (pm != pinnedm.end()) {
			continue;
		}

		int hop_time = ceil((float)msg.size/(float)msgSize);
		//int hop_time = msg.size;
		msg.injection_time = findandlockMsg (max((j->start_time+j->wcet_fullspeed),horizonStart), hop_time, msg.route, msg.id);
		msg.arrival_time = message(msg.id).arrival_time;
		//msg.injection_time = findandlockMsg (j.start_time+j.wcet_fullspeed, hop_time, msg.route);
/*		if (msg.route.size() == 0) {
			msg.arrival_time = msg.injection_time;
		} else {
			msg.arrival_time = msg.injection_time + ((msg.route.size()-1) * hop_time);
		}*/

		auto parent = find(msg.receiver, pinnedj);
		if (parent != pinnedj.end() && (msg.arrival_time>job(msg.receiver).start_time)) {
			return false;
		}
		message(msg.id) = msg;
	}

	return true;
}

Fractal::Schedule Fractal::Model::schedule(int *genes, vector<Job> pinnedj, vector<Message> pinnedm) {
	/* Some constants to provide easier access to the gene */
	const int kAllocOffs = 0;
	const int kRouteOffs = kAllocOffs + njobs_;
	const int kJobOrderOffs = kRouteOffs + nmsgs_;
	const int kMessageWeightOffs = kJobOrderOffs + njobs_;

	bool valid = true;

	//Construct a graph of the application model
	Graph applicationGraph (njobs_);
	jobLocks = {};
	msgLocks = {};

	//Update the jobs and messages based on the genes
	for (auto &job : jobs_) {
		job.runs_on = genes[kAllocOffs + job.id];
		job.weight = genes[kJobOrderOffs + job.id];
		job.wcet_with_compression = job.wcet_fullspeed;
		int fx = 0;
		float nu = 1.0 * frequencies_.at(fx) / frequencies_.back();
		job.freq_index = fx;
		job.nu = nu;
		job.wcet_at_freq = ceil(job.wcet_with_compression / nu);
	}

	for (auto &msg : msgs_) {
		//Add edge to job graph
		applicationGraph.addEdge(msg.sender, msg.receiver);
		msg.scheme_idx = 0;
		msg.compressed_size = msg.size;

		//Get sender and receiver jobs
		Fractal::Job sender = job(msg.sender);
		Fractal::Job receiver = job(msg.receiver);

		msg.weight = genes[kMessageWeightOffs+msg.id];

		if (is_pinned(pinnedm,msg.id)) {
			continue;
		}

		//Update the route index
		int route_idx = genes[kRouteOffs + msg.id];
		int routeRange = 4;
		int msgIndexRange = KSP[sender.runs_on][receiver.runs_on].size()-1;
		int routeIndex = ((route_idx*msgIndexRange)/routeRange);

		msg.route_idx = routeIndex;
		Route route{};
		if (sender.runs_on == receiver.runs_on) {
			msg.route = route;
			continue;
		}
		else {
			route = KSP[sender.runs_on][receiver.runs_on][msg.route_idx];
			//route.erase(route.begin());
			msg.route = route;
		}
	}
	int msgSize = 32;

	//Fix jobs and messages
	for (auto jb : pinnedj) {
		//Fractal::Job& j = job(jb.id);
		//j.start_time = findandlockJob(jb.start_time, jb.wcet_fullspeed, jb.runs_on);
		Lock lck = {jb.start_time, (jb.start_time+jb.wcet_fullspeed), jb.runs_on};
		jobLocks[jb.runs_on].push_back(lck);

	}

	for (auto mg : pinnedm) {
		if (mg.route.empty()) {
			continue;
		}
		//Fractal::Message &msg = message(mg.id);
		///int hop_time = ceil((float)mg.size/(float)msgSize);
		//int hop_time = mg.size;
		//msg.injection_time = findandlockMsg (mg.injection_time, hop_time, mg.route);
		//msg.arrival_time = mg.arrival_time;
		int injection = mg.injection_time;

		vector<int> lks;
		for (int i=0;i<mg.route.size()-1;i++) {
			int st = mg.route[i];
			int ed = mg.route[i+1];
			for (auto l:links_) {
				if ((l.start == st && l.end == ed) || (l.start == ed && l.end == st)) {
					lks.push_back(l.id);
				}
			}
		}

		int hop_time = ceil((float)mg.size/(float)msgSize);
		for (auto n : lks) {
			int delay = find(n,links_)->delay;
			Lock lck = {injection, (injection + (hop_time*delay)), n};
			msgLocks.push_back(lck);
			injection = injection + (hop_time*delay);
		}
	}
	//int check=1;
	//Compute allocation ordering of the jobs
	vector<int> jobAllocationOrder = applicationGraph.topologicalSort(&jobs_);
	for (auto jobID : jobAllocationOrder) {
		//Get the job
		Fractal::Job& j = job(jobID);
		auto parent = find(j.id, pinnedj);
		if (parent == pinnedj.end()) {
			int earliestStartTime = earliestTime (msgs_,j.id);
			j.start_time = findandlockJob(max(earliestStartTime, horizonStart), j.wcet_fullspeed, j.runs_on);
		}
		 valid = allocateMsgs(&j, pinnedm, pinnedj);
			//allocateMsgs(&j, pinnedm, pinnedj);
		//int earliestStartTime = earliestTime (msgs_,j.id);
		//j.start_time = findandlockJob(max(earliestStartTime, horizonStart), j.wcet_fullspeed, j.runs_on);
	}

/*	if (zu == 1) {
		for (auto o : msgLocks) {
			cout <<o.resource << ": " << o.set << ": " << o.release << endl;
		}
	}*/


	 //The makespan
	int makespan = 0;
	 //energy consumption
	float energy = 0.0;

	 //Final write-back and ouput preparation
	for (auto &job : jobs_) {
		int finish_time = job.start_time + job.wcet_fullspeed; //job.wcet_at_freq;
		// update the makespan
		makespan = max(makespan, finish_time);
	}

//	 the schedule
	Fractal::Schedule schedule(jobs_, msgs_, nodes_, links_);

//	 add objects to solution and return
	schedule.set_makespan(makespan);
	schedule.set_valid(valid);

	return schedule;
}


