#ifdef EDITOR

#include "EditorCmdParams.h"

// Cmd parser usage: https://github.com/mmahnic/argumentum

void EditorCmdParams::add_parameters(argumentum::ParameterConfig& params)
{
    EngineCmdParams::add_parameters(params);
    params
        .add_parameter(editorCfg, "--editor-cfg", "--e")
        .default_value(DEFAULT_CONFIG_FILEPATH)
        .nargs(1)
        .help("Config file path");
}

#endif // EDITOR
