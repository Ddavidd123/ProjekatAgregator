#pragma once

class Consumer {
private:
	int consumerId;
	double currentConsumption;

public:
	Consumer(int id);

	double generateConsumption();
	void sendConsumption(double value);
	int getId() const;
};