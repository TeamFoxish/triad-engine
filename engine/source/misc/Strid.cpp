#include "Strid.h"

Strid ToStrid(const std::string& str)
{
	return Strid(sid::string_info{str.c_str(), str.length()});
}
