#pragma once

#include <argumentum/argparse.h>

class EngineCmdParams : public argumentum::Options {
protected:
	void add_parameters(argumentum::ParameterConfig& params) override;

public:
	std::optional<std::string> projectCfg;
	std::optional<std::string> engineCfg;
	std::optional<std::string> startupScene;

	static constexpr const char* DEFAULT_PROJECT_FILEPATH = "config/project.cfg"; // TODO: remove when SampleProject is decomposed to a separate proj
	static constexpr const char* DEFAULT_CONFIG_FILEPATH = "config/engine.cfg";
};
