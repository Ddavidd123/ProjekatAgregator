#include "Agregator.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

Agregator::Agregator() {

}

void Agregator::initialize() {
	network.buildTree();
}

void Agregator::runAutomaticMode() {
	initialize();
	cout << "=== Automatski rezim ===\n";
	cout << "Rezim: AUTOMATIC - podaci se salju odmah\n";

}

void Agregator :: runBatchMode() {
	initialize();
	cout << "=== BATCH rezim ===\n";
	cout << "Rezim: BATCH - podaci se agriraju u intervalu\n";
}

void Agregator::printTotalConsumption() {
	Node* root = network.getRoot();
	if (root) {
		cout << "Ukupna potrosnja: " << root->getAggregatedConsumption() << " kWh\n";
	}
	else {
		cout << "Mreza nije inicijalizovana!\n";

	}
}
void Agregator::testWithSmallData() {
	cout << "=== Test sa malim brojem podataka ===\n";
	initialize();

	auto start = chrono::high_resolution_clock::now();
	std::cout << "Test sa malim brojem podataka - u toku...\n";
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Vreme izvršavanja: " << duration.count() << " ms\n";
}

void Agregator::testWithLargeData() {
	cout << "=== Test sa velikim brojem podataka (~10,000) ===\n";
	initialize();

	auto start = chrono::high_resolution_clock::now();
	std::cout << "Test sa velikim brojem podataka - u toku...\n";
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Vreme izvršavanja: " << duration.count() << " ms\n";
}