#include "Agregator.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

Agregator::Agregator() {

}

void Agregator::initialize() {
	// Obrisi staru mrezu pre kreiranja nove
	// Network destruktor ce to uradiiti automatski
	network = Network(); //kreira novu mrezu
	network.buildTree();
}

void Agregator::runAutomaticMode() {
	initialize();
	cout << "=== Automatski rezim ===\n";
	cout << "Rezim: AUTOMATIC - podaci se salju odmah\n";
	network.resetAllConsumptions();
	network.sendRequestToAll();

	printTotalConsumption();
}

void Agregator::runBatchMode() {
	initialize();
	cout << "=== BATCH rezim ===\n";
	
	// Postavi sve cvorove u BATCH rezim
	network.setAllNodesMode(OperationMode::BATCH);
	
	network.resetAllConsumptions();
	
	cout << "Sakupljanje podataka u intervalu od 3 sekunde...\n";
	
	// Sakupi podatke u intervalu (simulacija - generiši više puta)
	for (int i = 0; i < 5; i++) {
		network.sendRequestToAll();
		cout << "Generisanje podataka " << (i + 1) << "/5...\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(600)); // 600ms = 3s ukupno / 5
	}
	
	// Agregiraj sve i prikazi rezultat
	cout << "Agregacija podataka...\n";
	printTotalConsumption();
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
	cout << "=== Test sa malim brojem podataka (10 podataka) ===\n";
	initialize();

	auto start = std::chrono::high_resolution_clock::now();

	// Resetuj pre početka
	network.resetAllConsumptions();

	// Generiši 10 podataka
	for (int i = 0; i < 10; i++) {
		network.sendRequestToAll();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

	printTotalConsumption();
	cout << "Vreme izvrsavanja: " << duration.count() << " ms\n";
}

void Agregator::testWithLargeData() {
	cout << "=== Test sa velikim brojem podataka (~10,000) ===\n";
	initialize();

	auto start = std::chrono::high_resolution_clock::now();

	// Resetuj pre početka
	network.resetAllConsumptions();

	// Generiši ~10,000 podataka
	for (int i = 0; i < 10000; i++) {
		network.sendRequestToAll();
	}

	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start);

	printTotalConsumption();
	cout << "Vreme izvrsavanja: " << duration.count() << " ms\n";
}