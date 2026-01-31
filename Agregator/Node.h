#pragma once

#include "DynamicArray.h"
#include <mutex>
#include "Consumer.h"

// Režim rada za čvorove i potrošače
enum class OperationMode {
	AUTOMATIC, // svaki report se odmah prosleđuje nagore
	BATCH      // reportovi se sakupljaju i šalju u batch-u
};

class Node {
private:
	int nodeId;
	Node* parent;
	DynamicArray<Node*> children;
	double aggregatedConsumption;
	OperationMode mode;
	
	// za BATCH režim na nivou agregatora
	DynamicArray<double> pendingConsumptions; // lokalno primljeni, ali još ne prosleđeni nagore
	DynamicArray<Consumer*> nodeConsumers;    // potrošači (listovi) vezani za ovaj čvor

	mutable std::mutex mtx_;  // thread-safety za agregaciju i pending
	
public:
	Node(int id, OperationMode m = OperationMode::AUTOMATIC);
	
	void addChild(Node* child);
	void setParent(Node* p);
	
	// DATA UP: prijem potrošnje od child čvora ili potrošača
	void receiveConsumption(double value);
	
	// CONTROL DOWN: propagacija komande za merenje ka child čvorovima i potrošačima
	void requestConsumption();
	
	// Podešavanje režima rada i propagacija na potrošače
	void setMode(OperationMode m);
	void addConsumer(Consumer* consumer);
	
	int getId() const;
	Node* getParent();
	const Node* getParent() const;
	double getAggregatedConsumption() const;
	
	// Reset agregacije za novi test/interval
	void resetAggregation();
	
	// Za BATCH režim – flush batch podataka (potrošači i child čvorovi šalju nagore)
	void processBatch();
	
	// Prikaz strukture stabla (za debug/informacije)
	void printTreeStructure(int indent = 0) const;
};