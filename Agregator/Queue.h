#pragma once

#include <cstddef>
#include <functional>
#include <utility>

/**
 * Samostalna implementacija reda (FIFO) - povezana lista.
 * Bez STL struktura.
 */
class TaskQueue {
public:
	TaskQueue() : head_(nullptr), tail_(nullptr), size_(0) {}
	~TaskQueue() { clear(); }

	TaskQueue(const TaskQueue&) = delete;
	TaskQueue& operator=(const TaskQueue&) = delete;

	void push(std::function<void()> task) {
		Node* n = new Node{ std::move(task), nullptr };
		if (tail_) tail_->next = n;
		else head_ = n;
		tail_ = n;
		++size_;
	}

	bool pop(std::function<void()>& out) {
		if (!head_) return false;
		Node* n = head_;
		head_ = head_->next;
		if (!head_) tail_ = nullptr;
		out = std::move(n->task);
		delete n;
		--size_;
		return true;
	}

	bool empty() const { return head_ == nullptr; }
	size_t size() const { return size_; }

	void clear() {
		while (head_) {
			Node* n = head_;
			head_ = head_->next;
			delete n;
		}
		tail_ = nullptr;
		size_ = 0;
	}

private:
	struct Node {
		std::function<void()> task;
		Node* next;
	};
	Node* head_;
	Node* tail_;
	size_t size_;
};
