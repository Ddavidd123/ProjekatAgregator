#pragma once

#include "HashMap.h"

/**
 * Samostalna implementacija skupa celih brojeva.
 * Wrapper oko HashMap<int, bool> - bez std::set.
 */
class IntSet {
public:
	void insert(int value) { map_.insert(value, true); }
	void erase(int value) { map_.erase(value); }
	bool contains(int value) const { return map_.contains(value); }
	size_t size() const { return map_.size(); }
	bool empty() const { return map_.empty(); }
	void clear() { map_.clear(); }

	// Za iteraciju - jednostavno cuvanje u niz uz mapu; ovde koristimo samo contains/insert/erase
	// Agregator koristi registeredIds_.count(id) i .erase - to je contains i erase.
private:
	HashMap<int, bool> map_;
};
