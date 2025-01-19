#pragma once

#include "resource/Res.h"

class asITypeInfo;

class CNativeObject {
public:
	virtual void ApplyOverrides(const YAML::Node& overrides) {}

	virtual YAML::Node Serialize() const { return YAML::Node(); }

	static void RecognizeNativeType(asITypeInfo* type) { nativeObjects.insert(type); }

	static int IsTypeNative(asITypeInfo* type) { return nativeObjects.contains(type); }

private:
	static inline std::set<asITypeInfo*> nativeObjects;
};
