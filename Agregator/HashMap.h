#pragma once

#include "DynamicArray.h"
#include <cstddef>
#include <functional>

/**
 * Samostalna implementacija hash mape (otvoreno adresiranje).
 * Kljucevi int ili socket_t (preko cast-a u size_t).
 */
template<typename K, typename V>
class HashMap {
public:
	HashMap() : size_(0), capacity_(16) {
		keys_ = new K[capacity_];
		values_ = new V[capacity_];
		used_ = new bool[capacity_]();
		for (size_t i = 0; i < capacity_; ++i)
			used_[i] = false;
	}

	~HashMap() {
		delete[] keys_;
		delete[] values_;
		delete[] used_;
	}

	HashMap(const HashMap&) = delete;
	HashMap& operator=(const HashMap&) = delete;

	void insert(const K& key, const V& value) {
		if (size_ * 2 >= capacity_) rehash();
		size_t idx = findSlot(key);
		if (!used_[idx]) {
			used_[idx] = true;
			keys_[idx] = key;
			values_[idx] = value;
			++size_;
		} else {
			values_[idx] = value;
		}
	}

	bool find(const K& key, V& out) const {
		size_t idx = findSlotForKey(key);
		if (idx == (size_t)-1) return false;
		out = values_[idx];
		return true;
	}

	V* get(const K& key) {
		size_t idx = findSlotForKey(key);
		if (idx == (size_t)-1) return nullptr;
		return &values_[idx];
	}

	const V* get(const K& key) const {
		size_t idx = findSlotForKey(key);
		if (idx == (size_t)-1) return nullptr;
		return &values_[idx];
	}

	bool erase(const K& key) {
		size_t idx = findSlotForKey(key);
		if (idx == (size_t)-1) return false;
		// Ukloni element i rehash preostale (zbog otvorenog adresiranja)
		DynamicArray<K> kArr;
		DynamicArray<V> vArr;
		for (size_t i = 0; i < capacity_; ++i) {
			if (used_[i] && !(keys_[i] == key)) {
				kArr.push_back(keys_[i]);
				vArr.push_back(values_[i]);
			}
		}
		clear();
		for (size_t i = 0; i < kArr.size(); ++i)
			insert(kArr[i], vArr[i]);
		return true;
	}

	bool contains(const K& key) const {
		return findSlotForKey(key) != (size_t)-1;
	}

	size_t size() const { return size_; }
	bool empty() const { return size_ == 0; }

	void clear() {
		for (size_t i = 0; i < capacity_; ++i) used_[i] = false;
		size_ = 0;
	}

	template<typename Arr>
	void getValues(Arr& out) const {
		for (size_t i = 0; i < capacity_; ++i)
			if (used_[i]) out.push_back(values_[i]);
	}

private:
	static size_t hashInt(size_t h) {
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;
		return h;
	}

	size_t hash(const K& key) const {
		return hashInt(static_cast<size_t>(key));
	}

	size_t findSlot(const K& key) {
		size_t h = hash(key) % capacity_;
		size_t idx = h;
		do {
			if (!used_[idx]) return idx;
			if (keys_[idx] == key) return idx;
			idx = (idx + 1) % capacity_;
		} while (idx != h);
		return (size_t)-1;
	}

	size_t findSlotForKey(const K& key) const {
		size_t h = hash(key) % capacity_;
		size_t idx = h;
		do {
			if (!used_[idx]) return (size_t)-1;
			if (keys_[idx] == key) return idx;
			idx = (idx + 1) % capacity_;
		} while (idx != h);
		return (size_t)-1;
	}

	void rehash() {
		size_t oldCap = capacity_;
		K* oldKeys = keys_;
		V* oldVals = values_;
		bool* oldUsed = used_;

		capacity_ *= 2;
		keys_ = new K[capacity_];
		values_ = new V[capacity_];
		used_ = new bool[capacity_]();
		for (size_t i = 0; i < capacity_; ++i) used_[i] = false;
		size_ = 0;

		for (size_t i = 0; i < oldCap; ++i) {
			if (oldUsed[i])
				insert(oldKeys[i], oldVals[i]);
		}
		delete[] oldKeys;
		delete[] oldVals;
		delete[] oldUsed;
	}

	K* keys_;
	V* values_;
	bool* used_;
	size_t size_;
	size_t capacity_;
};
