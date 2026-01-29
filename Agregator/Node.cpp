#include "Node.h"
#include <iostream>

using namespace std;

Node::Node(int id, OperationMode m)
	: nodeId(id),
	  parent(nullptr),
	  aggregatedConsumption(0.0),
	  mode(m) {
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
	Node* p = nullptr;
	{
		lock_guard<mutex> lock(mtx_);
		aggregatedConsumption += value;
		if (mode == OperationMode::AUTOMATIC)
			p = parent;
		else
			pendingConsumptions.push_back(value);
	}
	if (p != nullptr)
		p->receiveConsumption(value);
}

void Node::requestConsumption() {
	// CONTROL DOWN: šalje komandu svim lokalnim potrošačima i child čvorovima
	for (Consumer* consumer : nodeConsumers) {
		consumer->handleRequest();
	}
	
	for (Node* child : children) {
		child->requestConsumption();
	}
}

void Node::setMode(OperationMode m) {
	mode = m;
	
	// Propagiraj režim i na potrošače ovog čvora
	for (Consumer* consumer : nodeConsumers) {
		consumer->setMode(m);
	}
}

int Node::getId() const {
	return nodeId;
}

Node* Node::getParent() {
	return parent;
}

const Node* Node::getParent() const {
	return parent;
}

double Node::getAggregatedConsumption() const {
	lock_guard<mutex> lock(mtx_);
	return aggregatedConsumption;
}

void Node::processBatch() {
	// 1) Potrošači šalju svoje batch izveštaje
	for (Consumer* consumer : nodeConsumers) {
		consumer->flushBatch();
	}
	
	// 2) Recimo i child čvorovi da izvrše svoj batch flush (rekurzivno)
	for (Node* child : children) {
		child->processBatch();
	}
	
	if (mode == OperationMode::BATCH) {
		double sum = 0.0;
		{
			lock_guard<mutex> lock(mtx_);
			for (double val : pendingConsumptions)
				sum += val;
			pendingConsumptions.clear();
		}
		if (parent != nullptr && sum > 0.0)
			parent->receiveConsumption(sum);
	}
}

void Node::addConsumer(Consumer* consumer)
{
	nodeConsumers.push_back(consumer);
	consumer->setParent(this);
	consumer->setMode(mode);
}

void Node::resetAggregation() {
	{
		lock_guard<mutex> lock(mtx_);
		aggregatedConsumption = 0.0;
		pendingConsumptions.clear();
	}
	for (Consumer* consumer : nodeConsumers)
		consumer->reset();
	for (Node* child : children)
		child->resetAggregation();
}

void Node::printTreeStructure(int indent) const {
	// Ispis indentacije
	for (int i = 0; i < indent; i++) {
		cout << "  ";
	}
	
	cout << "Node ID: " << nodeId;
	if (nodeId == 0)
		cout << " (Data Source - Drzava)";
	else if (nodeId >= 1 && nodeId <= 6)
		cout << " (Agr " << (nodeId - 1) << ")";
	else
		cout << " (Region)";
	
	// Ispis potrošača povezanih sa ovim čvorom
	if (!nodeConsumers.empty()) {
		cout << " -> Potrosaci: [";
		for (size_t i = 0; i < nodeConsumers.size(); i++) {
			cout << nodeConsumers[i]->getId();
			if (i < nodeConsumers.size() - 1) {
				cout << ", ";
			}
		}
		cout << "]";
	}
	cout << "\n";
	
	// Rekurzivno ispisuj child čvorove
	for (Node* child : children) {
		child->printTreeStructure(indent + 1);
	}
}