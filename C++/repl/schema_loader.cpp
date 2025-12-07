#include "schema_loader.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace ct {
Schema load_schema_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw SchemaError("Cannot open schema file: " + path);
  }
  std::ostringstream s;
  s << in.rdbuf();
  return parse_schema_text(s.str());
}
} // namespace ct
