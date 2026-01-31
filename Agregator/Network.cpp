#include "Network.h"
#include <iostream>

Network::Network() : root(nullptr) {
}

Network::~Network() {
	clear();
}

void Network::clear() {
	DynamicArray<Node*> nodes;
	allNodes.getValues(nodes);
	for (size_t i = 0; i < nodes.size(); ++i)
		delete nodes[i];
	allNodes.clear();
	for (size_t i = 0; i < consumers.size(); ++i)
		delete consumers[i];
	consumers.clear();
	consumerIdToParent_.clear();
	root = nullptr;
}

void Network::buildTree() {
	clear();
	// Arhitektura prema dijagramu: Data Source -> Agr 0 -> Agr 1,2 -> Agr 3,4,5 -> Data Destinations
	// Root = Data Source (drzava), id 0
	root = new Node(0, OperationMode::AUTOMATIC);
	allNodes.insert(0, root);

	Node* agr0 = new Node(1, OperationMode::AUTOMATIC);
	allNodes.insert(1, agr0);
	root->addChild(agr0);

	Node* agr1 = new Node(2, OperationMode::AUTOMATIC);
	Node* agr2 = new Node(3, OperationMode::AUTOMATIC);
	allNodes.insert(2, agr1);
	allNodes.insert(3, agr2);
	agr0->addChild(agr1);
	agr0->addChild(agr2);

	Node* agr3 = new Node(4, OperationMode::AUTOMATIC);
	Node* agr4 = new Node(5, OperationMode::AUTOMATIC);
	Node* agr5 = new Node(6, OperationMode::AUTOMATIC);
	allNodes.insert(4, agr3);
	allNodes.insert(5, agr4);
	allNodes.insert(6, agr5);
	agr1->addChild(agr3);
	agr1->addChild(agr4);
	agr2->addChild(agr5);

	// Data Destinations (potrosaci): ispod Agr 3, 4, 5
	Consumer* c1 = new Consumer(10);
	Consumer* c2 = new Consumer(11);
	Consumer* c3 = new Consumer(12);
	Consumer* c4 = new Consumer(13);
	Consumer* c5 = new Consumer(14);
	Consumer* c6 = new Consumer(15);
	consumers.push_back(c1);
	consumers.push_back(c2);
	consumers.push_back(c3);
	consumers.push_back(c4);
	consumers.push_back(c5);
	consumers.push_back(c6);

	agr3->addConsumer(c1);
	agr3->addConsumer(c2);
	agr4->addConsumer(c3);
	agr4->addConsumer(c4);
	agr5->addConsumer(c5);
	agr5->addConsumer(c6);

	consumerIdToParent_.insert(10, agr3);
	consumerIdToParent_.insert(11, agr3);
	consumerIdToParent_.insert(12, agr4);
	consumerIdToParent_.insert(13, agr4);
	consumerIdToParent_.insert(14, agr5);
	consumerIdToParent_.insert(15, agr5);
}

Node* Network::findNode(int nodeId) {
	Node* p = nullptr;
	return allNodes.find(nodeId, p) ? p : nullptr;
}

bool Network::nodeExists(int nodeId) const {
	return allNodes.contains(nodeId);
}

void Network::sendRequest(int targetNodeId) {
	Node* node = findNode(targetNodeId);
	if (node) {
		node->requestConsumption();
	}
	else {
		std::cout << "Node sa ID " << targetNodeId << " ne postoji u mrezi.\n";
	}
}

void Network::sendRequestToAll() {
	if (root) {
		root->requestConsumption();
	}
}

void Network::processBatches() {
	if (root) {
		root->processBatch();
	}
}

void Network::processBatchesFromNode(int targetNodeId) {
	Node* node = findNode(targetNodeId);
	if (node) {
		node->processBatch();
	}
	else {
		std::cout << "Node sa ID " << targetNodeId << " ne postoji u mrezi.\n";
	}
}

Node* Network::getRoot() const {
	return root;
}

const DynamicArray<Consumer*>& Network::getConsumers() const {
	return consumers;
}

Node* Network::getParentOfConsumer(int consumerId) const {
	Node* p = nullptr;
	return consumerIdToParent_.find(consumerId, p) ? p : nullptr;
}

bool Network::isValidConsumerId(int consumerId) const {
	return consumerIdToParent_.contains(consumerId);
}

bool Network::isConsumerInSubtree(int consumerId, int nodeId) const {
	if (nodeId == 0) return true;
	Node* p = getParentOfConsumer(consumerId);
	if (!p) return false;
	for (;;) {
		if (p->getId() == nodeId) return true;
		p = p->getParent();
		if (!p) return false;
	}
}

void Network::resetAllConsumptions() {
	if (root) {
		root->resetAggregation();
	}
}

void Network::setAllNodesMode(OperationMode mode) {
	DynamicArray<Node*> nodes;
	allNodes.getValues(nodes);
	for (size_t i = 0; i < nodes.size(); ++i)
		nodes[i]->setMode(mode);
}

void Network::printTreeStructure() const {
	if (root) {
		std::cout << "\n=== Struktura stabla (topologija mreze) ===\n";
		root->printTreeStructure(0);
		std::cout << "\nNapomena: nodeId za zahtev prema delu drzave: 0 (cela) | 1 Agr0 | 2 Agr1 | 3 Agr2 | 4 Agr3 | 5 Agr4 | 6 Agr5\n";
		std::cout << "==========================================\n\n";
	} else {
		std::cout << "Mreza nije inicijalizovana!\n";
	}
}