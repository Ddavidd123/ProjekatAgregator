#include "Node.h"
#include <iostream>

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
		std::lock_guard<std::mutex> lock(mtx_);
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
	for (size_t i = 0; i < nodeConsumers.size(); ++i)
		nodeConsumers[i]->handleRequest();
	
	for (size_t i = 0; i < children.size(); ++i)
		children[i]->requestConsumption();
}

void Node::setMode(OperationMode m) {
	mode = m;
	
	// Propagiraj režim i na potrošače ovog čvora
	for (size_t i = 0; i < nodeConsumers.size(); ++i)
		nodeConsumers[i]->setMode(m);
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
	std::lock_guard<std::mutex> lock(mtx_);
	return aggregatedConsumption;
}

void Node::processBatch() {
	// 1) Potrošači šalju svoje batch izveštaje
	for (size_t i = 0; i < nodeConsumers.size(); ++i)
		nodeConsumers[i]->flushBatch();
	
	// 2) Recimo i child čvorovi da izvrše svoj batch flush (rekurzivno)
	for (size_t i = 0; i < children.size(); ++i)
		children[i]->processBatch();
	
	if (mode == OperationMode::BATCH) {
		double sum = 0.0;
		{
			std::lock_guard<std::mutex> lock(mtx_);
			for (size_t i = 0; i < pendingConsumptions.size(); ++i)
				sum += pendingConsumptions[i];
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
		std::lock_guard<std::mutex> lock(mtx_);
		aggregatedConsumption = 0.0;
		pendingConsumptions.clear();
	}
	for (size_t i = 0; i < nodeConsumers.size(); ++i)
		nodeConsumers[i]->reset();
	for (size_t i = 0; i < children.size(); ++i)
		children[i]->resetAggregation();
}

void Node::printTreeStructure(int indent) const {
	// Ispis indentacije
	for (int i = 0; i < indent; i++)
		std::cout << "  ";
	
	std::cout << "Node ID: " << nodeId;
	if (nodeId == 0)
		std::cout << " (Data Source - Drzava)";
	else if (nodeId >= 1 && nodeId <= 6)
		std::cout << " (Agr " << (nodeId - 1) << ")";
	else
		std::cout << " (Region)";
	
	// Ispis potrošača povezanih sa ovim čvorom
	if (!nodeConsumers.empty()) {
		std::cout << " -> Potrosaci: [";
		for (size_t i = 0; i < nodeConsumers.size(); i++) {
			std::cout << nodeConsumers[i]->getId();
			if (i < nodeConsumers.size() - 1)
				std::cout << ", ";
		}
		std::cout << "]";
	}
	std::cout << "\n";
	
	// Rekurzivno ispisuj child čvorove
	for (size_t i = 0; i < children.size(); ++i)
		children[i]->printTreeStructure(indent + 1);
}