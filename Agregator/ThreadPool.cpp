#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads) {
	workers_.reserve(numThreads);
	for (size_t i = 0; i < numThreads; ++i)
		workers_.push_back(std::thread(&ThreadPool::worker, this));
}

ThreadPool::~ThreadPool() {
	stop_ = true;
	cvTask_.notify_all();
	for (auto& w : workers_)
		if (w.joinable()) w.join();
}

void ThreadPool::submit(std::function<void()> task) {
	{
		std::lock_guard<std::mutex> lock(mtx_);
		tasks_.push(std::move(task));
		++pending_;
	}
	cvTask_.notify_one();
}

void ThreadPool::waitAll() {
	std::unique_lock<std::mutex> lock(mtx_);
	cvDone_.wait(lock, [this] { return pending_ == 0; });
}

void ThreadPool::worker() {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(mtx_);
			cvTask_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
			if (stop_ && tasks_.empty()) return;
			tasks_.pop(task);
		}
		task();
		{
			std::lock_guard<std::mutex> lock(mtx_);
			--pending_;
			cvDone_.notify_all();
		}
	}
}
