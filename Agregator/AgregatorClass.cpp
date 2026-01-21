#include "Agregator.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

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

	Node* root = network.getRoot();
	double total = root ? root->getAggregatedConsumption() : 0.0;

	printTotalConsumption();
	cout << "Vreme izvrsavanja: " << duration.count() << " ms\n";

	// Dokumentovanje u fajl
	std::ofstream log("TestResults.txt", std::ios::app);
	if (log.is_open()) {
		log << "TestWithSmallData (AUTOMATIC default)\n";
		log << "Broj zahteva: 10\n";
		log << "Ukupna potrosnja: " << total << " kWh\n";
		log << "Vreme izvrsavanja: " << duration.count() << " ms\n";
		log << "----------------------------------------\n";
	}
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

	Node* root = network.getRoot();
	double total = root ? root->getAggregatedConsumption() : 0.0;

	printTotalConsumption();
	cout << "Vreme izvrsavanja: " << duration.count() << " ms\n";

	// Dokumentovanje u fajl
	std::ofstream log("TestResults.txt", std::ios::app);
	if (log.is_open()) {
		log << "TestWithLargeData (AUTOMATIC default)\n";
		log << "Broj zahteva: 10000\n";
		log << "Ukupna potrosnja: " << total << " kWh\n";
		log << "Vreme izvrsavanja: " << duration.count() << " ms\n";
		log << "----------------------------------------\n";
	}
}

// Testiranje oba režima (AUTOMATIC i BATCH) za mali broj podataka
void Agregator::testAllModesSmall() {
	cout << "=== Test SA MALIM brojem podataka (10) za oba rezima ===\n\n";

	// AUTOMATIC
	cout << "[AUTOMATIC] Pokretanje testa sa 10 zahteva...\n";
	initialize();
	network.setAllNodesMode(OperationMode::AUTOMATIC);
	network.resetAllConsumptions();

	auto startAuto = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10; i++) {
		network.sendRequestToAll();
	}
	auto endAuto = std::chrono::high_resolution_clock::now();
	auto durationAuto = chrono::duration_cast<chrono::milliseconds>(endAuto - startAuto);

	Node* rootAuto = network.getRoot();
	double totalAuto = rootAuto ? rootAuto->getAggregatedConsumption() : 0.0;

	printTotalConsumption();
	cout << "[AUTOMATIC] Vreme izvrsavanja: " << durationAuto.count() << " ms\n\n";

	// BATCH
	cout << "[BATCH] Pokretanje testa sa 10 zahteva...\n";
	initialize();
	network.setAllNodesMode(OperationMode::BATCH);
	network.resetAllConsumptions();

	auto startBatch = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10; i++) {
		network.sendRequestToAll();
	}
	auto endBatch = std::chrono::high_resolution_clock::now();
	auto durationBatch = chrono::duration_cast<chrono::milliseconds>(endBatch - startBatch);

	Node* rootBatch = network.getRoot();
	double totalBatch = rootBatch ? rootBatch->getAggregatedConsumption() : 0.0;

	printTotalConsumption();
	cout << "[BATCH] Vreme izvrsavanja: " << durationBatch.count() << " ms\n";

	// Dokumentovanje u fajl
	std::ofstream log("TestResults.txt", std::ios::app);
	if (log.is_open()) {
		log << "TestAllModesSmall\n";
		log << "Broj zahteva: 10\n";
		log << "[AUTOMATIC] Ukupna potrosnja: " << totalAuto << " kWh, vreme: " << durationAuto.count() << " ms\n";
		log << "[BATCH]     Ukupna potrosnja: " << totalBatch << " kWh, vreme: " << durationBatch.count() << " ms\n";
		log << "----------------------------------------\n";
	}
}

// Testiranje oba režima (AUTOMATIC i BATCH) za veliki broj podataka
void Agregator::testAllModesLarge() {
	cout << "=== Test SA VELIKIM brojem podataka (~10,000) za oba rezima ===\n\n";

	// AUTOMATIC
	cout << "[AUTOMATIC] Pokretanje testa sa 10.000 zahteva...\n";
	initialize();
	network.setAllNodesMode(OperationMode::AUTOMATIC);
	network.resetAllConsumptions();

	auto startAuto = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; i++) {
		network.sendRequestToAll();
	}
	auto endAuto = std::chrono::high_resolution_clock::now();
	auto durationAuto = chrono::duration_cast<chrono::milliseconds>(endAuto - startAuto);

	Node* rootAuto = network.getRoot();
	double totalAuto = rootAuto ? rootAuto->getAggregatedConsumption() : 0.0;

	printTotalConsumption();
	cout << "[AUTOMATIC] Vreme izvrsavanja: " << durationAuto.count() << " ms\n\n";

	// BATCH
	cout << "[BATCH] Pokretanje testa sa 10.000 zahteva...\n";
	initialize();
	network.setAllNodesMode(OperationMode::BATCH);
	network.resetAllConsumptions();

	auto startBatch = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; i++) {
		network.sendRequestToAll();
	}
	auto endBatch = std::chrono::high_resolution_clock::now();
	auto durationBatch = chrono::duration_cast<chrono::milliseconds>(endBatch - startBatch);

	Node* rootBatch = network.getRoot();
	double totalBatch = rootBatch ? rootBatch->getAggregatedConsumption() : 0.0;

	printTotalConsumption();
	cout << "[BATCH] Vreme izvrsavanja: " << durationBatch.count() << " ms\n";

	// Dokumentovanje u fajl
	std::ofstream log("TestResults.txt", std::ios::app);
	if (log.is_open()) {
		log << "TestAllModesLarge\n";
		log << "Broj zahteva: 10000\n";
		log << "[AUTOMATIC] Ukupna potrosnja: " << totalAuto << " kWh, vreme: " << durationAuto.count() << " ms\n";
		log << "[BATCH]     Ukupna potrosnja: " << totalBatch << " kWh, vreme: " << durationBatch.count() << " ms\n";
		log << "----------------------------------------\n";
	}
}