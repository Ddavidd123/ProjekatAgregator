#pragma once

#include "DynamicArray.h"
#include "Queue.h"
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
 * Samostalna implementacija thread poola.
 * Koristi DynamicArray i TaskQueue - bez STL struktura.
 */
class ThreadPool {
public:
	explicit ThreadPool(size_t numThreads);
	~ThreadPool();
	void submit(std::function<void()> task);
	void waitAll();

private:
	void worker();

	DynamicArray<std::thread> workers_;
	TaskQueue tasks_;
	std::mutex mtx_;
	std::condition_variable cvTask_;
	std::condition_variable cvDone_;
	std::atomic<bool> stop_{ false };
	std::atomic<int> pending_{ 0 };
};
