# WP4T42-07 Hierarchical Metascheduler

The hierarchical metascheduler is an offline tool that invokes a scheduler (Genetic Algorithm) repeatedly to compute adapted schedules for each runtime context event. The inputs to the metascheduler consist of the application model describing the computational jobs and communication messages for a given application, the platform model describing the system architecture on which the application is executed, and the context model describing all runtime context events pertinent to the application and platform. 

Runtime context events within a system, either at the FRACTAL node level or in a set of FRACTAL nodes, are adapted to for energy efficiency and fault recovery by optimizing the system resource allocation (scheduling). The computed schedules allow for runtime adaptation of the system to context events.

## Getting Started

These instructions will guide the user on setting up the hierarchical metascheduler on a local machine and computing a multi-schedule graph (MSG) for any application (AM), platform (PM) and context model (CM).

The hierarchical metascheduler relies on three input files; the AM/PM describes the jobs and messages of the application, the computational nodes, routers and platform interconnection (links). The CM describes the runtime context events (slack, failure, thermal events, etc.) that are monitored for. The hierarchical metascheduler computes an adapted schedule for these context events in the CM.

The third file "Parameters.txt", describes the input files and the genetic algorithm configuration. 

For instructions on generating the AM/PM [application/platform model example](source/example_N2.json) see WP4T42-03.

CM [context model example](source/context10.json).

### Prerequisites

C++ compiler

### Installation

```sh
$ Clone repository				#	get local copy of source files
$ cd path/to/repository/source			#	change directory to source directory
$ make						#	compile source files
```

## Usage

```sh
$ Parameters.txt -> AM, PM, CM		#	set input files 
$ ./scheduler				#	run hierarchical metascheduler
```

All computed schedules are saved in the results folder [example computed schedule](source/results/schedule--0.json).

## Deployment/Interface

The hierarchical metascheduler is an independent offline tool to compute adapted schedules for time-triggered systems. These adapted schedules are deployed at runtime for adaptation. 

### Data Structure

* C++ data structures

## Additional Documentation and Acknowledgments

* GA library [galib](http://lancet.mit.edu/ga/)
