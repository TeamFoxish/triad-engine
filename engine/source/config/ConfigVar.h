#pragma once

#include <string>
#include <string_view>

template<typename T>
class ConfigVar {
public:
	ConfigVar(std::string_view path, const T& defaultVal);
	ConfigVar(const ConfigVar&) = delete;
	ConfigVar(ConfigVar&&) = delete;

	// TODO: add Get() with no reference creation?
	const T& GetRef() const { return QueryVal(); }
	operator T() const { return GetRef(); }

private:
	const T& QueryVal() const;

private:
	std::string path;
	mutable T cachedVal;
	mutable bool isCached = false;
};
