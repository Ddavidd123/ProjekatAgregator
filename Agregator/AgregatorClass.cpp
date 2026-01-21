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
	
	// CONTROL DOWN: zahtev ka celom stablu
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
		// CONTROL DOWN: komanda za merenje (potrošači akumuliraju u BATCH režimu)
		network.sendRequestToAll();
		cout << "Generisanje podataka " << (i + 1) << "/5...\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(600)); // 600ms = 3s ukupno / 5
	}
	
	// Na kraju intervala: DATA UP – flush svih batch izveštaja nagore
	cout << "Agregacija podataka...\n";
	network.processBatches();
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

void Agregator::testAllModesSmall() {
	cout << "=== Test SA MALIM brojem podataka (10) u BATCH rezimu ===\n\n";

	cout << "[BATCH] Pokretanje testa sa 10 zahteva...\n";
	initialize();
	network.setAllNodesMode(OperationMode::BATCH);
	network.resetAllConsumptions();

	auto startBatch = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10; i++) {
		// Potrošači i čvorovi sakupljaju podatke u BATCH režimu
		network.sendRequestToAll();
	}
	// Na kraju intervala flush svih batch izveštaja nagore
	network.processBatches();
	auto endBatch = std::chrono::high_resolution_clock::now();
	auto durationBatch = chrono::duration_cast<chrono::milliseconds>(endBatch - startBatch);

	Node* rootBatch = network.getRoot();
	double totalBatch = rootBatch ? rootBatch->getAggregatedConsumption() : 0.0;

	printTotalConsumption();
	cout << "[BATCH] Vreme izvrsavanja: " << durationBatch.count() << " ms\n";

	// Dokumentovanje u fajl
	std::ofstream log("TestResults.txt", std::ios::app);
	if (log.is_open()) {
		log << "TestBatchSmall\n";
		log << "Broj zahteva: 10\n";
		log << "[BATCH] Ukupna potrosnja: " << totalBatch << " kWh, vreme: " << durationBatch.count() << " ms\n";
		log << "----------------------------------------\n";
	}
}

void Agregator::testAllModesLarge() {
	cout << "=== Test SA VELIKIM brojem podataka (~10,000) u BATCH rezimu ===\n\n";

	cout << "[BATCH] Pokretanje testa sa 10.000 zahteva...\n";
	initialize();
	network.setAllNodesMode(OperationMode::BATCH);
	network.resetAllConsumptions();

	auto startBatch = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; i++) {
		// Potrošači i čvorovi sakupljaju podatke u BATCH režimu
		network.sendRequestToAll();
	}
	// Na kraju intervala flush svih batch izveštaja nagore
	network.processBatches();
	auto endBatch = std::chrono::high_resolution_clock::now();
	auto durationBatch = chrono::duration_cast<chrono::milliseconds>(endBatch - startBatch);

	Node* rootBatch = network.getRoot();
	double totalBatch = rootBatch ? rootBatch->getAggregatedConsumption() : 0.0;

	printTotalConsumption();
	cout << "[BATCH] Vreme izvrsavanja: " << durationBatch.count() << " ms\n";

	// Dokumentovanje u fajl
	std::ofstream log("TestResults.txt", std::ios::app);
	if (log.is_open()) {
		log << "TestBatchLarge\n";
		log << "Broj zahteva: 10000\n";
		log << "[BATCH] Ukupna potrosnja: " << totalBatch << " kWh, vreme: " << durationBatch.count() << " ms\n";
		log << "----------------------------------------\n";
	}
}

void Agregator::sendRequestToAll() {
	initialize();
	cout << "=== Slanje zahteva svim potrosacima (celokupna drzava) ===\n";
	
	// Podrazumevano AUTOMATIC režim
	network.setAllNodesMode(OperationMode::AUTOMATIC);
	network.resetAllConsumptions();
	
	network.sendRequestToAll();
	
	printTotalConsumption();
}

void Agregator::sendRequestToSubtree(int nodeId) {
	initialize();
	
	// Prvo prikaži strukturu stabla da korisnik vidi koje ID-jeve može koristiti
	printTreeStructure();
	
	cout << "=== Slanje zahteva delu drzave (subtree) od nodeId = " << nodeId << " ===\n";
	
	// Podrazumevano AUTOMATIC režim za ovaj zahtev
	network.setAllNodesMode(OperationMode::AUTOMATIC);
	network.resetAllConsumptions();
	
	network.sendRequest(nodeId);
	
	// Ako je subtree u BATCH režimu, korisnik može ručno pozvati i processBatchesFromNode,
	// ali za AUTOMATIC režim nije potrebno.
	printTotalConsumption();
}

void Agregator::printTreeStructure() {
	initialize();
	network.printTreeStructure();
}