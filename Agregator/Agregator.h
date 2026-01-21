#pragma once

#include "Network.h"

class Agregator {
private:
	Network network;

public:
	Agregator();

	void initialize();
	void runAutomaticMode();
	void runBatchMode();
	void printTotalConsumption();
	
	// Slanje zahteva:
	//  - svim potrošačima (celom stablu)
	//  - samo jednom delu države (subtree, od nodeId naniže)
	void sendRequestToAll();
	void sendRequestToSubtree(int nodeId);

	void testWithSmallData();
	void testWithLargeData();
	void testAllModesSmall();
	void testAllModesLarge();
	
	// Prikaz strukture stabla (hijerarhija čvorova i potrošača)
	void printTreeStructure();
};