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

	void testWithSmallData();
	void testWithLargeData();
	void testAllModesSmall();
	void testAllModesLarge();
};