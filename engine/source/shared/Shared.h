#pragma once

#include "resource/Res.h"

class asITypeInfo;

class CNativeObject {
public:
	virtual void ApplyOverrides(const YAML::Node& overrides) {}

	virtual YAML::Node Serialize() const { return YAML::Node(); }

	static void RecognizeNativeType(asITypeInfo* type) { nativeObjects.insert(type); }

	static int IsTypeNative(const asITypeInfo* type) { return nativeObjects.contains(type); }

public:
	// used to determine which scalar type for inputs should imgui draw
	static constexpr const char* DOUBLE_TAG = "double"; // used by default
	static constexpr const char* INT_TAG = "int";
	static constexpr const char* STR_TAG = "str";

private:
	static inline std::set<const asITypeInfo*> nativeObjects;
};
