#include "Consumer.h"
#include <random>
using namespace std;

Consumer::Consumer(int id) : consumerId(id), currentConsumption(0.0)
{

}

double Consumer :: generateConsumption() {
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(10.0, 100.0);
	return dis(gen);
}

void Consumer :: sendConsumption(double value) {
	//za sada samo cuvamo vrednost
	currentConsumption = value;

}

int Consumer :: getId() const {
	return consumerId;
}