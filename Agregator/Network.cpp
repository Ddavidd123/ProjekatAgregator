#include "Network.h"
#include <iostream>
using namespace std;

Network::Network() : root(nullptr) {

}

Network::~Network() {
	clear();
}

void Network::clear() {
	for (auto& p : allNodes)
		delete p.second;
	allNodes.clear();
	for (Consumer* c : consumers)
		delete c;
	consumers.clear();
	consumerIdToParent_.clear();
	root = nullptr;
}

void Network::buildTree() {
	clear();
	// Arhitektura prema dijagramu: Data Source -> Agr 0 -> Agr 1,2 -> Agr 3,4,5 -> Data Destinations
	// Root = Data Source (drzava), id 0
	root = new Node(0, OperationMode::AUTOMATIC);
	allNodes[0] = root;

	Node* agr0 = new Node(1, OperationMode::AUTOMATIC);
	allNodes[1] = agr0;
	root->addChild(agr0);

	Node* agr1 = new Node(2, OperationMode::AUTOMATIC);
	Node* agr2 = new Node(3, OperationMode::AUTOMATIC);
	allNodes[2] = agr1;
	allNodes[3] = agr2;
	agr0->addChild(agr1);
	agr0->addChild(agr2);

	Node* agr3 = new Node(4, OperationMode::AUTOMATIC);
	Node* agr4 = new Node(5, OperationMode::AUTOMATIC);
	Node* agr5 = new Node(6, OperationMode::AUTOMATIC);
	allNodes[4] = agr3;
	allNodes[5] = agr4;
	allNodes[6] = agr5;
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

	consumerIdToParent_[10] = agr3;
	consumerIdToParent_[11] = agr3;
	consumerIdToParent_[12] = agr4;
	consumerIdToParent_[13] = agr4;
	consumerIdToParent_[14] = agr5;
	consumerIdToParent_[15] = agr5;
}

Node* Network::findNode(int nodeId) {
	auto it = allNodes.find(nodeId);
	if (it != allNodes.end())
		return it->second;
	return nullptr;
}

bool Network::nodeExists(int nodeId) const {
	return allNodes.find(nodeId) != allNodes.end();
}

void Network::sendRequest(int targetNodeId) {
	Node* node = findNode(targetNodeId);
	if (node) {
		node->requestConsumption();
	}
	else {
		cout << "Node sa ID " << targetNodeId << " ne postoji u mrezi.\n";
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
		cout << "Node sa ID " << targetNodeId << " ne postoji u mrezi.\n";
	}
}

Node* Network::getRoot() const {
	return root;
}

const std::vector<Consumer*>& Network::getConsumers() const {
	return consumers;
}

Node* Network::getParentOfConsumer(int consumerId) const {
	auto it = consumerIdToParent_.find(consumerId);
	return it != consumerIdToParent_.end() ? it->second : nullptr;
}

bool Network::isValidConsumerId(int consumerId) const {
	return consumerIdToParent_.find(consumerId) != consumerIdToParent_.end();
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
	for (auto pair : allNodes) {
		pair.second->setMode(mode);
	}
}

void Network::printTreeStructure() const {
	if (root) {
		cout << "\n=== Struktura stabla (topologija mreze) ===\n";
		root->printTreeStructure(0);
		cout << "\nNapomena: nodeId za zahtev prema delu drzave: 0 (cela) | 1 Agr0 | 2 Agr1 | 3 Agr2 | 4 Agr3 | 5 Agr4 | 6 Agr5\n";
		cout << "==========================================\n\n";
	} else {
		cout << "Mreza nije inicijalizovana!\n";
	}
}