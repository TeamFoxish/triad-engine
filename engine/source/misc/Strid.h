#pragma once

#include <config.hpp>

#undef STRID_DEBUG
#ifdef DEBUG
#define STRID_DEBUG 1
#else
#define STRID_DEBUG 0
#endif

#include <string>
#include <string_id.hpp>
#include <database.hpp>

namespace sid = foonathan::string_id;
using Strid = sid::string_id<sid::default_database>;

// allows the use of string literal operators "***"_id, "***"_id64
using namespace sid::literals;

inline Strid ToStrid(const std::string& str);
