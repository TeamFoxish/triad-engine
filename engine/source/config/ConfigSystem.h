#pragma once

#include <string_view>
#include <iostream>

// TODO: somehow move away this include. until then never include ConfigSystem.h in any file except Main.cpp
#include "tao/config.hpp"

class ConfigSystem {
public:
	ConfigSystem(std::string_view cfgFilePath);
	ConfigSystem(const ConfigSystem&) = delete;
	ConfigSystem(ConfigSystem&&) = delete;
	~ConfigSystem();

	template <typename T>
	T GetValue(std::string_view path, const T& defaultVal) const;

public:
	static std::optional<ConfigSystem*> Instance() { return instance; }

protected:
	tao::config::value config;

private:
	static inline ConfigSystem* instance = nullptr;
};

template<typename T>
inline T ConfigSystem::GetValue(std::string_view path, const T& defaultVal) const
{
	try {
		const tao::json::pointer p(path.data());
		const tao::config::value& val = config.at(p);
		return val.as<T>();
	} catch (std::exception ex) {
		std::cerr << ex.what() << ": " << path << '\n';
	}
	return defaultVal;
}
