#include "Network.h"
using namespace std;

Network::Network() : root(nullptr) {

}

Network::~Network() {
	for (auto pair : allNodes) {
		delete pair.second;

	}
	allNodes.clear();

	//obrisi sve potrosace
	for (Consumer* consumer : consumers) {
		delete consumer;
	}
	consumers.clear();
	root = nullptr;
}

void Network::buildTree() {

	// Resetuj prethodne podatke ako postoje
	if (root != nullptr) {
		// Obri�i staru mre�u (opciono - za sada samo kreiramo novu)
	}

	//Kreiraj root cvor (drzava)
	root = new Node(0, OperationMode::AUTOMATIC);
	allNodes[0] = root;

	//kreiraj nekoliko regiona(srednji cvor)
	Node* node1 = new Node(1, OperationMode::AUTOMATIC);
	Node * node2 = new Node(1, OperationMode::AUTOMATIC);
	allNodes[1] = node1;
	allNodes[2] = node2;
	root->addChild(node1);
	root->addChild(node2);

	//Kreiraj potrosace i dodeli ih cvorovima
	Consumer* c1 = new Consumer(10);
	Consumer* c2 = new Consumer(11);
	Consumer* c3 = new Consumer(12);
	Consumer* c4 = new Consumer(13);

	consumers.push_back(c1);
	consumers.push_back(c2);
	consumers.push_back(c3);
	consumers.push_back(c4);


	node1->addConsumer(c1);
	node1->addConsumer(c2);
	node2->addConsumer(c3);
	node2->addConsumer(c4);
}

Node* Network::findNode(int nodeId) {
	auto it = allNodes.find(nodeId);
	if (it != allNodes.find(nodeId)) {
		return it->second;
	}
	return nullptr;
}

void Network::sendRequest(int targetNodeId) {
	Node* node = findNode(targetNodeId);
	if (node) {
		node->requestConsumption();
	}
}

void Network::sendRequestToAll() {
	if (root) {
		root->requestConsumption();
	}
}

Node* Network::getRoot() const {
	return root;
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