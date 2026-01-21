#include "Consumer.h"
#include "Node.h"
#include <random>

Consumer::Consumer(int id, OperationMode m)
    : consumerId(id),
      currentConsumption(0.0),
      parent(nullptr),
      mode(m),
      batchAccumulated(0.0) {
}

double Consumer::generateConsumption() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(10.0, 100.0);
    return dis(gen);
}

void Consumer::setParent(Node* p) {
    parent = p;
}

void Consumer::setMode(OperationMode m) {
    mode = m;
    // novi režim -> resetuj batch akumulaciju
    batchAccumulated = 0.0;
}

void Consumer::handleRequest() {
    double value = generateConsumption();
	
    if (mode == OperationMode::AUTOMATIC) {
        // Svako merenje odmah šalje parentu kao jedan report
        currentConsumption = value;
        if (parent) {
            parent->receiveConsumption(value);
        }
    }
    else { // BATCH
        // Sakuplja merenja u intervalu
        batchAccumulated += value;
        currentConsumption = batchAccumulated;
    }
}

void Consumer::flushBatch() {
    if (mode == OperationMode::BATCH && batchAccumulated > 0.0) {
        if (parent) {
            parent->receiveConsumption(batchAccumulated);
        }
        batchAccumulated = 0.0;
    }
}

void Consumer::reset() {
    currentConsumption = 0.0;
    batchAccumulated = 0.0;
}

void Consumer::sendConsumption(double value) {
    // Ostavljeno za eventualnu buduću upotrebu;
    // trenutno se slanje vrši kroz handleRequest/flushBatch.
    currentConsumption = value;
    if (mode == OperationMode::AUTOMATIC && parent) {
        parent->receiveConsumption(value);
    }
}

int Consumer::getId() const {
    return consumerId;
}