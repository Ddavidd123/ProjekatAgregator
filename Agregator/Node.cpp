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
		
		//ne salje odma ceka da se podaci prikupe
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
	// NE resetuj ovde - resetuje se samo pre početka testa kroz resetAllConsumptions()
	// Ovde samo akumuliraj nove podatke
	
	// Prvo traži potrošnju od potrošača i dodaj u agregaciju
	for (Consumer* consumer : nodeConsumers) {
		double consumption = consumer->generateConsumption();
		aggregatedConsumption += consumption;
	}

	// Zatim traži od child čvorova (rekurzivno)
	for (Node* child : children) {
		double childBefore = child->getAggregatedConsumption(); // Zabeleži pre
		child->requestConsumption();
		double childAfter = child->getAggregatedConsumption(); // Zabeleži posle
		// Dodaj samo NOVE podatke (razliku)
		aggregatedConsumption += (childAfter - childBefore);
	}
	
	// U AUTOMATIC režimu, root ne šalje ništa (nema parent)
	// Child čvorovi ne treba da šalju jer parent direktno uzima njihove podatke
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

void Node::resetAggregation() {
	aggregatedConsumption = 0.0;
	pendingConsumptions.clear();

	// Resetuj i child čvorove rekurzivno
	for (Node* child : children) {
		child->resetAggregation();
	}
}