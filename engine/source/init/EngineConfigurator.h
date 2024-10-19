#pragma once

#include <memory>

class EngineCmdParams;
class EngineRuntime;

class EngineConfigurator {
public:
	bool CreateRuntime(int argc, char* argv[], std::unique_ptr<EngineRuntime>& runtime);
	bool CreateRuntime(const EngineCmdParams& params, std::unique_ptr<EngineRuntime>&runtime);
};
