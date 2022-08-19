/*
 * metascheduler.hpp
 *
 *  Created on: Apr 28, 2021
 *      Author: pascal
 */

#ifndef METASCHEDULER_HPP_
#define METASCHEDULER_HPP_

#include <fstream>
#include <chrono>

#include <vector>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <math.h>

#include "schedule.hpp"
#include "scheduler.hpp"
#include "MsGraph.h"
#include "Calendar.h"
#include "model.hpp"
#include "context.hpp"

#include <galib/ga/ga.h>
#include <galib/ga/std_stream.h>
//#include "mpi.h"

using chrono::high_resolution_clock;
using chrono::duration_cast;
using chrono::duration;
using chrono::milliseconds;

namespace Fractal {
void createMSG(Fractal::Model *model, Fractal::Context *context);
}
#endif /* METASCHEDULER_HPP_ */
