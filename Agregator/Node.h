#pragma once

#include <vector>
#include "Consumer.h"
using namespace std;

enum class OperationMode {
	AUTOMATIC,
	BATCH
};

class Node {
private:
	int nodeId;
	Node* parent;
	vector<Node*> children;
	double aggregatedConsumption;
	OperationMode mode;
	
	//za BATCH rezim
	vector<double> pendingConsumptions;
	vector<Consumer*> nodeConsumers;

public:
	Node(int id, OperationMode m = OperationMode::AUTOMATIC);

	void addChild(Node* child);
	void setParent(Node* p);
	void receiveConsumption(double value);
	void aggregate();
	void sendToParent();
	void requestConsumption();
	void setMode(OperationMode m);
	void addConsumer(Consumer* consumer);
	int getId() const;
	double getAggregatedConsumption() const;

	//Za BATCH rezim
	void processBatch();



};