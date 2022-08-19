/*
 * Calendar.cpp
 *
 *  Created on: May 6, 2021
 *      Author: pascal
 */

#include "Calendar.h"

namespace Fractal {

void Calendar::addEvent(Event& event) {
	listOfEvents.push_back(event);
}

void Calendar::updateEvents(Schedule *schedule) {
	for (Event& E : listOfEvents) {
		switch (E.eventType) {
		case SLACK:
			E.eventTime = schedule->job(E.target).start_time + E.value;
			break;

		case FAULT:
			E.eventTime = (E.value * schedule->makespan()) / 100;
			break;
		}
	}
}

void Calendar::sortCalendar() {
	listOfEvents.sort(
			[](const Fractal::Event& event1, const Fractal::Event& event2) {
				return event1.eventTime < event2.eventTime;
			});
}

void Calendar::printCalendar() {
	cout << " 	Events in Calendar	 :" << endl;
	for_each(listOfEvents.begin(), listOfEvents.end(),
			[](const Fractal::Event& event) {
				cout << "EventID: " << event.ID << " :: " << "Target: " << event.target << " :: " << "Event at: " << event.eventTime << endl;
			});
}

void Calendar::removeEvent(int ID){
	auto event = find_if(listOfEvents.begin(), listOfEvents.end(), [ID](Event& event){
		return event.ID == ID;
	});

	if (event!=listOfEvents.end()){
		listOfEvents.erase(event);
	}
}
} /* namespace Fractal */
