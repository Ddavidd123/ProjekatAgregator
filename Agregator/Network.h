#pragma once

#include <map>
#include <vector>
#include "Node.h"
#include "Consumer.h"
using namespace std;

class Network {
private:
	Node* root;
	map<int, Node*> allNodes;
	vector<Consumer*> consumers;

public:
	Network();
	~Network();

	void buildTree();
	Node* findNode(int nodeId);
	void sendRequest(int targetNodeId);
	void sendRequestToAll();
	void resetAllConsumptions();
	void setAllNodesMode(OperationMode mode);
	Node* getRoot() const;
};