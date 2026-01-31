#pragma once

#include <cstddef>
#include <cstring>
#include <utility>

/**
 * Samostalna implementacija dinamickog niza (analogon std::vector).
 * Koristi samo raw niz i rucnu alokaciju - bez STL.
 */
template<typename T>
class DynamicArray {
public:
	DynamicArray() : data_(nullptr), size_(0), capacity_(0) {}
	~DynamicArray() { clear(); }

	DynamicArray(const DynamicArray& other) : data_(nullptr), size_(0), capacity_(0) {
		reserve(other.size_);
		for (size_t i = 0; i < other.size_; ++i)
			new (data_ + i) T(other.data_[i]);
		size_ = other.size_;
	}

	DynamicArray& operator=(const DynamicArray& other) {
		if (this != &other) {
			clear();
			reserve(other.size_);
			for (size_t i = 0; i < other.size_; ++i)
				new (data_ + i) T(other.data_[i]);
			size_ = other.size_;
		}
		return *this;
	}

	DynamicArray(DynamicArray&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
		other.data_ = nullptr;
		other.size_ = other.capacity_ = 0;
	}

	void push_back(const T& value) {
		if (size_ >= capacity_) grow();
		new (data_ + size_) T(value);
		++size_;
	}

	void push_back(T&& value) {
		if (size_ >= capacity_) grow();
		new (data_ + size_) T(static_cast<T&&>(value));
		++size_;
	}

	void pop_back() {
		if (size_ > 0) {
			data_[size_ - 1].~T();
			--size_;
		}
	}

	void erase(size_t index) {
		if (index >= size_) return;
		for (size_t i = index; i < size_ - 1; ++i)
			data_[i] = std::move(data_[i + 1]);
		data_[size_ - 1].~T();
		--size_;
	}

	void clear() {
		for (size_t i = 0; i < size_; ++i)
			data_[i].~T();
		delete[] reinterpret_cast<char*>(data_);
		data_ = nullptr;
		size_ = capacity_ = 0;
	}

	void reserve(size_t n) {
		if (n <= capacity_) return;
		T* newData = reinterpret_cast<T*>(new char[n * sizeof(T)]);
		for (size_t i = 0; i < size_; ++i) {
			new (newData + i) T(static_cast<T&&>(data_[i]));
			data_[i].~T();
		}
		delete[] reinterpret_cast<char*>(data_);
		data_ = newData;
		capacity_ = n;
	}

	void resize(size_t n) {
		if (n > capacity_) reserve(n);
		while (size_ < n) {
			new (data_ + size_) T();
			++size_;
		}
		while (size_ > n) {
			--size_;
			data_[size_].~T();
		}
	}

	size_t size() const { return size_; }
	bool empty() const { return size_ == 0; }

	T& operator[](size_t i) { return data_[i]; }
	const T& operator[](size_t i) const { return data_[i]; }

	T* data() { return data_; }
	const T* data() const { return data_; }

	T& back() { return data_[size_ - 1]; }
	const T& back() const { return data_[size_ - 1]; }

	T* begin() { return data_; }
	T* end() { return data_ + size_; }
	const T* begin() const { return data_; }
	const T* end() const { return data_ + size_; }

private:
	void grow() {
		size_t newCap = (capacity_ == 0) ? 4 : capacity_ * 2;
		reserve(newCap);
	}

	T* data_;
	size_t size_;
	size_t capacity_;
};
