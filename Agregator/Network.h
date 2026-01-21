#pragma once

#include <map>
#include <vector>
#include "Node.h"
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
	
	// Slanje komande NADOLE
	void sendRequest(int targetNodeId);   // ka odabranom subtree-ju
	void sendRequestToAll();              // ka celom stablu (od root-a)
	
	// Obrada BATCH režima (SLANJE PODATAKA NAGORE)
	void processBatches();                // sa root-a (cela mreža)
	void processBatchesFromNode(int targetNodeId); // samo odabran subtree
	
	void resetAllConsumptions();
	void setAllNodesMode(OperationMode mode);
	Node* getRoot() const;
	
	// Prikaz strukture stabla (hijerarhija čvorova i potrošača)
	void printTreeStructure() const;
};