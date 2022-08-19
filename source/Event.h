/*
 * Event.h
 *
 *  Created on: May 6, 2021
 *      Author: pascal
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <vector>
#include <list>
#include <boost/assign/list_of.hpp>
#include <bits/stdc++.h>
#include <boost/unordered_map.hpp>
#include <iostream>
#include "utils/json.hpp"

using boost::assign::map_list_of;

using namespace std;
using json = nlohmann::json;

static int     seq_nr;

enum Event_type { INVALID_EVENT, SLACK, JOBFINISH, FAULT, BATTERY};
enum faultSubCategory {NO_SUB, CRASH, BABBLING_IDIOT, OMISSION};

const boost::unordered_map<Event_type,const char*> Event_typeToString = map_list_of
	(INVALID_EVENT, "Nil")
    (SLACK, "Slack")
    (FAULT, "Failure");

const boost::unordered_map<faultSubCategory,const char*> faultSubCategoryToString = map_list_of
	(NO_SUB, "0")
	(CRASH, "Crash");

namespace Fractal {

class Event {
public:
	int ID;
	int target;
	int eventTime;
	int value;
	int old;
	Event_type eventType;
	faultSubCategory subCategory;

	//Comparison operator to compare events
    bool operator == (const Event& s) const { return ID == s.ID && target == s.target && eventTime == s.eventTime && value == s.value && eventType == s.eventType && subCategory == s.subCategory; }
    //bool operator != (const Event& s) const { return !operator==(s); }

	Event(int id) : ID(seq_nr++), target(id), value(0), eventTime(0), old(0), subCategory(NO_SUB), eventType(INVALID_EVENT){};
	static void free_events();

	void setID(int ID);
	int getID();

	void setEventTime(int time);
	int getEventTime();
	void setEventType(Event_type evenType);
	Event_type getEventType();

	json tojson() {
		json event = json::object();
		//event["id"] = ID;
		event["source_id"] = target;
		event["context_type"] = Event_typeToString.at(eventType);
		//event["category"] = faultSubCategoryToString.at(subCategory);
		event["event_time"] = eventTime;
		switch (eventType) {
		case SLACK: {
			float slack = 100 - (((float)value/old) * 100);
			int tmp = (slack + .005) * 100;
			event["new_value"] = tmp/100.0;
			break;
		}

		case FAULT:
			event["new_value"] = value;
			break;
		}
		return event;
	}
};

} /* namespace Fractal */

#endif /* EVENT_H_ */
