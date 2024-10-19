#pragma once

#include <string_view>
#include <iostream>

#include <nlohmann/json.hpp>

class ConfigSystem {
public:
	ConfigSystem();
	ConfigSystem(const ConfigSystem&) = delete;
	ConfigSystem(ConfigSystem&&) = delete;
	~ConfigSystem();

	bool Init(std::string_view cfgFilePath);

	bool Override(std::string_view cfgFilePath);

	template <typename T>
	T GetValue(std::string_view path, const T& defaultVal) const;

public:
	static ConfigSystem* Instance() { return instance; }

protected:
	using json = nlohmann::json;
	static bool ReadConfigFile(std::string_view path, json& out);

protected:
	json config;

private:
	static inline ConfigSystem* instance = nullptr;
};

template<typename T>
inline T ConfigSystem::GetValue(std::string_view path, const T& defaultVal) const
{
	try {
		const json::json_pointer p(path.data());
		const json& val = config.at(p);
		return val.template get<T>();
	} catch (json::exception ex) {
		std::cerr << ex.what() << ": " << path << std::endl;
	}
	return defaultVal;
}
