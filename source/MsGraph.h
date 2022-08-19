/*
 * MsGraph.h
 *
 *  Created on: May 6, 2021
 *      Author: pascal
 */

#ifndef MSGRAPH_H_
#define MSGRAPH_H_

#include <list>
#include "MsgNode.h"
#include "chartdir.h"

namespace Fractal {

class MsGraph {
public:
	int numOfNodes(){return VectorOfNodes.size();}
	void addNode (MsgNode* msgNode){VectorOfNodes.push_back (msgNode);}
	void exportMSG ();
	void visualiseSchedules();
	vector <MsgNode*>& nodes(){return VectorOfNodes;}
	bool dejavu(MsgNode *newNode);
	//Schedule& getSchedule(int ID){return *VectorOfNodes.;};

private:
	vector <MsgNode*> VectorOfNodes;
};

} /* namespace Fractal */

#endif /* MSGRAPH_H_ */
