#include "ConfigSystem.h"

#include <iostream>

ConfigSystem::ConfigSystem(std::string_view cfgFilePath)
{
	try {
		config = tao::config::from_file(cfgFilePath);
	} catch (std::exception ex) {
		std::cerr << ex.what() << ": " << cfgFilePath << '\n';
		exit(EXIT_FAILURE);
		return;
	}
	assert(instance == nullptr);
	instance = this;
}

ConfigSystem::~ConfigSystem()
{
	assert(instance);
	instance = nullptr;
}
