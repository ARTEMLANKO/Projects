#pragma once
#include "schema_parser.h"

#include <sstream>

namespace ct {
Schema load_schema_file(const std::string& path);
} // namespace ct
