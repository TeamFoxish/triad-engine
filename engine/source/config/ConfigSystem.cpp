#include "ConfigSystem.h"

#include <iostream>
#include <fstream>
#include <sstream>

ConfigSystem::ConfigSystem()
{
	assert(instance == nullptr);
	instance = this;
}

ConfigSystem::~ConfigSystem()
{
	assert(instance);
	instance = nullptr;
}

bool ConfigSystem::Init(std::string_view cfgFilePath)
{
	assert(instance);
	if (!ReadConfigFile(cfgFilePath, config)) {
		return false;
	}
	return true;
}

bool ConfigSystem::Override(std::string_view cfgFilePath)
{
	assert(instance);
	json overrides;
	if (!ReadConfigFile(cfgFilePath, overrides)) {
		return false;
	}
	config.update(overrides, true);
	return true;
}

bool ConfigSystem::ReadConfigFile(std::string_view path, json& out)
{
	std::ifstream fJson(path.data());
	std::stringstream buffer;
	buffer << fJson.rdbuf();
	try {
		out = json::parse(buffer.str());
	} catch (const json::parse_error& ex) {
		std::cerr << "config parse error: " << ex.what() << ' ' << path << std::endl;
		return false;
	}
	return true;
}
