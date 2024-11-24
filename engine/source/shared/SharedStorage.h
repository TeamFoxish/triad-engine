#pragma once

#include "TransformStorage.h"

#include <memory>

struct SharedStorage {
	friend bool InitSharedStorage();
	friend void TermSharedStorage();

	SharedStorage() = default;
	SharedStorage(const SharedStorage&) = delete;
	SharedStorage(SharedStorage&&) = delete;
	~SharedStorage() = default;

	static SharedStorage& Instance() { return *instance; }

	TransformStorage transforms;

private:
	static inline std::unique_ptr<SharedStorage> instance;
};

bool InitSharedStorage();
void TermSharedStorage();
