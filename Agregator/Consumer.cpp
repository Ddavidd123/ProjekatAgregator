#include "Consumer.h"
#include <random>

Consumer::Consumer(int id) : consumerId(id), currentConsumption(0.0) {
}

double Consumer::generateConsumption() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(10.0, 100.0);
    return dis(gen);
}

void Consumer::sendConsumption(double value) {
    currentConsumption = value;
}

int Consumer::getId() const {
    return consumerId;
}