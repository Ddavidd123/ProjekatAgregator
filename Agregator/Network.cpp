#include "Network.h"
#include <iostream>
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
		// Stara mreža će biti obrisana u destruktoru Network objekta
	}
	
	//Kreiraj root cvor (drzava)
	root = new Node(0, OperationMode::AUTOMATIC);
	allNodes[0] = root;
	
	//kreiraj nekoliko regiona(srednji cvor)
	Node* node1 = new Node(1, OperationMode::AUTOMATIC);
	Node* node2 = new Node(2, OperationMode::AUTOMATIC); // ispravljeno: node2 ima id = 2
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
	if (it != allNodes.end()) { // BUGFIX: prethodno je uslov uvek bio false
		return it->second;
	}
	return nullptr;
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
		cout << "\nNapomena: Unesite nodeId (0, 1, ili 2) za slanje zahteva odredjenom delu drzave.\n";
		cout << "  - ID 0: cela drzava (root)\n";
		cout << "  - ID 1: region 1 (potrosaci 10, 11)\n";
		cout << "  - ID 2: region 2 (potrosaci 12, 13)\n";
		cout << "==========================================\n\n";
	} else {
		cout << "Mreza nije inicijalizovana!\n";
	}
}