/*
 * MsgNode.h
 *
 *  Created on: May 6, 2021
 *      Author: pascal
 */

#ifndef MSGNODE_H_
#define MSGNODE_H_

#include <list>
#include "model.hpp"
#include "context.hpp"
#include "schedule.hpp"
#include "Calendar.h"
#include "Event.h"

namespace Fractal {

class MsgNode {
public:
	MsgNode(int ID, Schedule* schedule, vector<Event> events) : id(ID), nodeSchedule(schedule), edge(events){};

	int getID() {return id;}
	Schedule *getSchedule() {return nodeSchedule;}
	void addChild(MsgNode *next) {children.push_back(next);}
	void saveCalendar (Calendar *cal) {nodeCalendar=*cal;}
	Calendar getCalendar() {return nodeCalendar;}
	vector<MsgNode*> getChildren(){return children;}
	vector<Event> getEdge() {return edge;}
private:
	int id;
	Schedule* nodeSchedule;
	vector<Event> edge;
	vector<MsgNode*> children;
	Calendar nodeCalendar;
};

} /* namespace Fractal */

#endif /* MSGNODE_H_ */
