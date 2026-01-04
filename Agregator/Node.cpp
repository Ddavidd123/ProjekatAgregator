#include "Node.h"

using namespace std;

Node::Node(int id, OperationMode m) : nodeId(id), parent(nullptr), aggregatedConsumption(0.0), mode(m) {

}

void Node::addChild(Node* child)
{
	children.push_back(child);
	child->setParent(this);
}

void Node::setParent(Node* p) {
	parent = p;
}

void Node::receiveConsumption(double value) {
	if (mode == OperationMode::AUTOMATIC) {
		aggregatedConsumption += value;
		sendToParent();
	}
	else { //BATCH mode
		pendingConsumptions.push_back(value);
		
	}
}

void Node::aggregate() {
	aggregatedConsumption = 0.0;
	for (double val : pendingConsumptions) {
		aggregatedConsumption += val;

	}
	for (Node* child : children)
	{
		aggregatedConsumption += child->getAggregatedConsumption();
	}
}

void Node::sendToParent() {
	if (parent != nullptr)
	{
		parent->receiveConsumption(aggregatedConsumption);
	}
}

void Node::requestConsumption() {
	//prvo trazi potrosnju od potrosaca
	for (Consumer* consumer : nodeConsumers) {
		double consumption = consumer->generateConsumption();
		receiveConsumption(consumption);
	}

	//zatim trazi od child cvorova
	for (Node* child : children)
	{
		child->requestConsumption();
	}
}

void Node::setMode(OperationMode m) {
	mode = m;
}

int Node::getId() const {
	return nodeId;
}

double Node::getAggregatedConsumption() const {
	return aggregatedConsumption;
}

void Node::processBatch() {
	aggregate();
	sendToParent();
	pendingConsumptions.clear();
}
void Node::addConsumer(Consumer* consumer)
{
	nodeConsumers.push_back(consumer);
}