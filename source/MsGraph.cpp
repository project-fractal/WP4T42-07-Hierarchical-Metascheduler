/*
 * MsGraph.cpp
 *
 *  Created on: May 6, 2021
 *      Author: pascal
 */

#include "MsGraph.h"
#include <list>

namespace Fractal {

void MsGraph::exportMSG() {
	//create .dot file for Graphviz
	ofstream msGraph("MSGraph.dot");
	msGraph << "digraph G {";

	//for every node in msg, write node and edge in file
	for (Fractal::MsgNode *node : VectorOfNodes) {
		string currentNode;
		currentNode = "S" + to_string(node->getID());

		for (Fractal::MsgNode* C : node->getChildren()){
			string child;
			child = "S" + to_string(C->getID());
			msGraph << currentNode + "->" + child;
			msGraph << " [label=";

			for (auto e : C->getEdge()) {
				msGraph << Event_typeToString.at(e.eventType);
				msGraph << ",";
			}
			msGraph << "]";
			msGraph << ";";
			msGraph << endl;
		}
	}
	msGraph << "}";
	//close .dot file
	msGraph.close();
}

void MsGraph::visualiseSchedules() {
}

bool MsGraph::dejavu(MsgNode *newNode){
	//If the node has an empty calendar, return.
	if (newNode->getCalendar().getEvents().empty())
		return false;

	/*Compare the node calendar to each calendar in each node in the MSG.
	If there is a match, set child nodes to that of the match,
	add the node to the MSG and return dejavu!*/
	for (Fractal::MsgNode *node : VectorOfNodes) {
		if (node->getCalendar().getEvents() == newNode->getCalendar().getEvents()){
			for (Fractal::MsgNode *child : node->getChildren()){
				newNode->addChild(child);
			}
			VectorOfNodes.push_back(newNode);
			return true;
		}
	}
	return false;
}
} /* namespace Fractal */
