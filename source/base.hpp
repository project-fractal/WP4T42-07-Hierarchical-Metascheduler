#ifndef BASE_HPP
#define BASE_HPP

#include <vector>
#include <string>
#include <numeric>

#include "json_tools.hpp"

#define WINDOW_SZ 1000

using Route = std::vector<int>;
using Order = std::vector<int>;

namespace Fractal {
  struct Message {
    int id;		// message id
    int size;		// size of the message
    int receiver;		// target job
    int sender;		// source job
    bool timetriggered;	// true <-> is time triggered
    
    int compressed_size;	// size of compressed message
    int scheme_idx;		// index of the compression scheme
    int injection_time;		// instant of transmission
    int route_idx;		// index of the route
    int arrival_time;
    int weight;
    Route route;

    Message (const int id,
	     const int size,
	     const int sender,
	     const int receiver,
	     const bool timetriggered)
      : id (id)
      , size (size)
      , receiver (receiver)
      , sender (sender)
      , timetriggered (timetriggered)
      , route_idx (0)
      , injection_time (0)
      , arrival_time (0)
    {}
    
    nlohmann::json tojson ()
    {
      nlohmann::json message = nlohmann::json::object ();
      message["id"] = id;
      message["size"] = size;
      message["compressed_size"] = compressed_size;
      message["scheme_idx"] = scheme_idx;
      message["injection_time"] = injection_time;
      message["route"] = route;
      message["route_idx"] = route_idx;
      message["timetriggered"] = timetriggered;
      message["sender"] = sender;
      message["receiver"] = receiver;
      message["arrival_time"] = arrival_time;
      return message;
    }
  };

  struct Job {
    int	id;		 // the jobs id
    int	wcet_fullspeed;	 // worst case exec time
    int	deadline;	 // deadline for this job
    int	max_temperature; // maximum temperature
    int mcet;		 // mean-case execution time
    int weight;		//competitiveness against adjacent jobs
    int pin = 0;
    
    std::vector<int>	can_run_on;	// list of nodes this job can run on
    int			runs_on={};	// the node the job is allocated to
    int			start_time;	// instant the job starts
    int			wcet_with_compression;
    int			wcet_at_freq;
    int			freq_index;
    float		nu;	// slow-down

    Job (const int id,
    	 const int wcet_fullspeed,
	 const int mcet,
    	 const int deadline,
    	 const int max_temperature,
	 std::vector<int> can_run_on)
      : id (id)
      , wcet_fullspeed (wcet_fullspeed)
      , mcet (mcet)
      , deadline (deadline)
      , max_temperature (max_temperature)
      , can_run_on (can_run_on)
      , weight (0)
    {}

    nlohmann::json tojson ()
    {
      nlohmann::json job = nlohmann::json::object ();
      job["id"] = id;
      job["wcet"] = wcet_fullspeed;
      job["deadline"] = deadline;
      job["wcet_with_compression"] = wcet_with_compression;
      job["runs_on"] = runs_on;
      job["start_time"] = start_time;
      job["can_run_on"] = can_run_on;
      job["frequency"] = freq_index;
      job["weight"] = weight;
      return job;
    }
  };

  struct Node {
    int id;		// the nodes id
    bool is_router;	// true <=> node is a router
    int temperature;	// current temperature

    Node (const int id,
	  const bool is_router,
	  int temperature)
      : id (id)
      , is_router (is_router)
      , temperature (temperature)
    {}

    nlohmann::json tojson ()
    {
      nlohmann::json node = nlohmann::json::object ();
      node["id"] = id;
      node["is_router"] = is_router;
      node["temperature"] = temperature;
      return node;
    }
  };

  struct Link {
    int id;			// the links id
    int start;			// a node connected...
    int end;			// ...to another node
    int delay;

    Link (const int id,
	  const int start,
	  const int end, const int delay)
      : id (id), delay(delay)
    {
      int lo = std::min (start, end);
      int hi = std::max (start, end);

      this->start = lo;
      this->end = hi;
    }

    nlohmann::json tojson ()
    {
      nlohmann::json link = nlohmann::json::object ();
      link["id"] = id;
      link["start"] = start;
      link["end"] = end;
      link["delay"] = delay;
      return link;
    }
  };

  struct Scheme {
    int		id;		// the schemes id
    int		wcdt;		// WCET of decompression
    int		wcct;		// WCET of compression
    float	wccr;		// the compression ratio
    
    Scheme (const int id,
	    const int wcdt,
	    const int wcct,
	    const float wccr)
      : id (id)
      , wcdt (wcdt)
      , wcct (wcct)
      , wccr (wccr)
    {}

    nlohmann::json tojson ()
    {
      nlohmann::json scheme = nlohmann::json::object ();
      scheme["id"] = id;
      scheme["wcdt"] = wcdt;
      scheme["wcct"] = wcct;
      scheme["wccr"] = wccr;
      return scheme;
    }
  };
} // namespace Dakodis
#endif
