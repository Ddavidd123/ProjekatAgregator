#include "Consumer.h"
#include "Node.h"
#include <random>
#include <mutex>

using namespace std;

Consumer::Consumer(int id, OperationMode m)
    : consumerId(id),
      currentConsumption(0.0),
      parent(nullptr),
      mode(m),
      batchAccumulated(0.0) {
}

double Consumer::generateConsumption() {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(10.0, 100.0);
    return dis(gen);
}

void Consumer::setParent(Node* p) {
    parent = p;
}

void Consumer::setMode(OperationMode m) {
    lock_guard<mutex> lock(mtx_);
    mode = m;
    batchAccumulated = 0.0;
}

void Consumer::handleRequest() {
    double value = generateConsumption();
    Node* p = nullptr;
    {
        lock_guard<mutex> lock(mtx_);
        if (mode == OperationMode::AUTOMATIC) {
            currentConsumption = value;
            p = parent;
        }
        else {
            batchAccumulated += value;
            currentConsumption = batchAccumulated;
        }
    }
    if (p)
        p->receiveConsumption(value);
}

void Consumer::flushBatch() {
    Node* p = nullptr;
    double val = 0.0;
    {
        lock_guard<mutex> lock(mtx_);
        if (mode == OperationMode::BATCH && batchAccumulated > 0.0) {
            val = batchAccumulated;
            batchAccumulated = 0.0;
            p = parent;
        }
    }
    if (p)
        p->receiveConsumption(val);
}

void Consumer::reset() {
    lock_guard<mutex> lock(mtx_);
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