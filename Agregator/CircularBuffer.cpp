#include "CircularBuffer.h"
#include <chrono>
#include <new>

CircularBuffer::CircularBuffer(size_t capacity) : capacity_(capacity > 0 ? capacity : 1) {
	data_ = new ConsumptionReport[capacity_];
}

CircularBuffer::~CircularBuffer() {
	delete[] data_;
}

bool CircularBuffer::push(const ConsumptionReport& item) {
	std::unique_lock<std::mutex> lock(mtx_);
	if (count_ >= capacity_) return false;
	data_[tail_] = item;
	tail_ = (tail_ + 1) % capacity_;
	++count_;
	cvNotEmpty_.notify_one();
	return true;
}

bool CircularBuffer::pop(ConsumptionReport& item) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (count_ == 0) return false;
	item = data_[head_];
	head_ = (head_ + 1) % capacity_;
	--count_;
	return true;
}

bool CircularBuffer::popWait(ConsumptionReport& item, int timeoutMs) {
	std::unique_lock<std::mutex> lock(mtx_);
	if (timeoutMs > 0) {
		if (!cvNotEmpty_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return count_ > 0 || done_; }))
			return false;
	} else {
		cvNotEmpty_.wait(lock, [this] { return count_ > 0 || done_; });
	}
	if (count_ == 0) return false;
	item = data_[head_];
	head_ = (head_ + 1) % capacity_;
	--count_;
	return true;
}

size_t CircularBuffer::size() const {
	std::lock_guard<std::mutex> lock(mtx_);
	return count_;
}

void CircularBuffer::setDone() {
	std::lock_guard<std::mutex> lock(mtx_);
	done_ = true;
	cvNotEmpty_.notify_all();
}

bool CircularBuffer::isDone() const {
	std::lock_guard<std::mutex> lock(mtx_);
	return done_;
}
