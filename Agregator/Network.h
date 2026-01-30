#pragma once

#include <unordered_map>
#include <vector>
#include "Node.h"
using namespace std;

class Network {
private:
	Node* root;
	unordered_map<int, Node*> allNodes;
	vector<Consumer*> consumers;
	unordered_map<int, Node*> consumerIdToParent_;  // za server: consumerId -> nadredjeni cvor
	
public:
	Network();
	~Network();
	Network(const Network&) = delete;
	Network& operator=(const Network&) = delete;

	void clear();
	void buildTree();
	Node* findNode(int nodeId);
	bool nodeExists(int nodeId) const;
	
	// Slanje komande NADOLE
	void sendRequest(int targetNodeId);   // ka odabranom subtree-ju
	void sendRequestToAll();              // ka celom stablu (od root-a)
	
	// Obrada BATCH režima (SLANJE PODATAKA NAGORE)
	void processBatches();                // sa root-a (cela mreža)
	void processBatchesFromNode(int targetNodeId); // samo odabran subtree
	
	void resetAllConsumptions();
	void setAllNodesMode(OperationMode mode);
	Node* getRoot() const;
	const std::vector<Consumer*>& getConsumers() const;
	Node* getParentOfConsumer(int consumerId) const;
	bool isValidConsumerId(int consumerId) const;
	bool isConsumerInSubtree(int consumerId, int nodeId) const;

	// Prikaz strukture stabla (hijerarhija čvorova i potrošača)
	void printTreeStructure() const;
};