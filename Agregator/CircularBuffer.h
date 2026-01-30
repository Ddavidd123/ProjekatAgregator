#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>

struct ConsumptionReport {
	int consumerId;
	double value;
};

class CircularBuffer {
public:
	explicit CircularBuffer(size_t capacity);
	bool push(const ConsumptionReport& item);
	bool pop(ConsumptionReport& item);
	bool popWait(ConsumptionReport& item, int timeoutMs);
	size_t size() const;
	void setDone();
	bool isDone() const;

private:
	std::vector<ConsumptionReport> data_;
	size_t head_ = 0;
	size_t tail_ = 0;
	size_t count_ = 0;
	size_t capacity_;
	mutable std::mutex mtx_;
	std::condition_variable cvNotEmpty_;
	bool done_ = false;
};
