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
	// Svaki report se agregira lokalno
	aggregatedConsumption += value;
	
	if (mode == OperationMode::AUTOMATIC) {
		// U AUTOMATIC režimu, svaki primljeni report se odmah prosleđuje parentu
		if (parent != nullptr) {
			parent->receiveConsumption(value);
		}
	}
	else { // BATCH mode
		// U BATCH režimu, samo akumuliramo u lokalni pending spisak,
		// prosleđivanje nagore se radi u processBatch()
		pendingConsumptions.push_back(value);
	}
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

double Node::getAggregatedConsumption() const {
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
	
	// 3) Ako je ovaj čvor u BATCH režimu, saberi lokalne pending vrednosti
	//    i pošalji ih parentu kao objedinjeni izveštaj
	if (mode == OperationMode::BATCH) {
		double sum = 0.0;
		for (double val : pendingConsumptions) {
			sum += val;
		}
		
		if (parent != nullptr && sum > 0.0) {
			parent->receiveConsumption(sum);
		}
		
		pendingConsumptions.clear();
	}
}

void Node::addConsumer(Consumer* consumer)
{
	nodeConsumers.push_back(consumer);
	consumer->setParent(this);
	consumer->setMode(mode);
}

void Node::resetAggregation() {
	aggregatedConsumption = 0.0;
	pendingConsumptions.clear();
	
	// Resetuj lokalne potrošače
	for (Consumer* consumer : nodeConsumers) {
		consumer->reset();
	}
	
	// Resetuj i child čvorove rekurzivno
	for (Node* child : children) {
		child->resetAggregation();
	}
}

void Node::printTreeStructure(int indent) const {
	// Ispis indentacije
	for (int i = 0; i < indent; i++) {
		cout << "  ";
	}
	
	// Ispis informacija o čvoru
	cout << "Node ID: " << nodeId;
	if (nodeId == 0) {
		cout << " (ROOT - Drzava)";
	} else {
		cout << " (Region)";
	}
	
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