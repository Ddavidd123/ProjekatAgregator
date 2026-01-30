#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
	explicit ThreadPool(size_t numThreads);
	~ThreadPool();
	void submit(std::function<void()> task);
	void waitAll();

private:
	void worker();

	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> tasks_;
	std::mutex mtx_;
	std::condition_variable cvTask_;
	std::condition_variable cvDone_;
	std::atomic<bool> stop_{ false };
	std::atomic<int> pending_{ 0 };
};
