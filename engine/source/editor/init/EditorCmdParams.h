#pragma once

#ifdef EDITOR

#include "init/EngineCmdParams.h"

// TODO: replace inheritance with composition in case of need
class EditorCmdParams : public EngineCmdParams {
protected:
	void add_parameters(argumentum::ParameterConfig& params) override;

public:
	std::optional<std::string> editorCfg;

	static constexpr const char* DEFAULT_CONFIG_FILEPATH = "config/editor.cfg";
};

#endif // EDITOR
