/*
 * Calendar.h
 *
 *  Created on: May 6, 2021
 *      Author: pascal
 */

#ifndef CALENDAR_H_
#define CALENDAR_H_

#include <list>
#include <iterator>
#include <cstddef>

#include "context.hpp"

using namespace std;

namespace Fractal {

class Calendar {
public:
	list <Event> getEvents(){return listOfEvents;}
	void addEvent (Event& event);
	void updateEvents (Schedule* schedule);
	void sortCalendar ();
	void printCalendar ();
	void removeEvent (int ID);
	void clearCalendar() {listOfEvents.clear();}

private:
	list <Event> listOfEvents;
};
} /* namespace Fractal */

#endif /* CALENDAR_H_ */
