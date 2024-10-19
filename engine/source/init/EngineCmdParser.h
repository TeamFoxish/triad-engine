#pragma once

#include <memory>
#include <argumentum/argparse.h>

struct EngineCmdParser {
    template<typename T>
	static bool Parse(int argc, char* argv[], const std::shared_ptr<T>& params);
};

template<typename T>
inline bool EngineCmdParser::Parse(int argc, char* argv[], const std::shared_ptr<T>& params)
{
    argumentum::argument_parser parser;
    parser.config().program(argv[0]);
    parser.params().add_parameters(params);
    if (!parser.parse_args(argc, argv)) {
        return false;
    }
    return true;
}
