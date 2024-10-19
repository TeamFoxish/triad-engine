#include "EngineCmdParams.h"

// Cmd parser usage: https://github.com/mmahnic/argumentum

void EngineCmdParams::add_parameters(argumentum::ParameterConfig& params)
{
    params
        .add_parameter(projectCfg, "--project", "--p")
        .default_value(DEFAULT_PROJECT_FILEPATH)
        .nargs(1)
        .help("Project file path");
    params
        .add_parameter(engineCfg, "--config", "--cfg")
        .default_value(DEFAULT_CONFIG_FILEPATH)
        .nargs(1)
        .help("Config file path");
    params
        .add_parameter(startupScene, "--scene", "--scn")
        .nargs(1)
        .help("Scene to load at startup");
}
