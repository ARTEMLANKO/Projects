#include "my_types.h"

namespace ct {

Type Type::builtin_of(Builtin b) {
  return Type{b, std::nullopt};
}

Type Type::user_of(std::string_view b) {
  return Type{std::nullopt, std::string(b)};
}

std::string Type::str() const {
  if (builtin) {
    if (*builtin == Builtin::Int32) {
      return "int32";
    }
    if (*builtin == Builtin::Int64) {
      return "int64";
    }
    if (*builtin == Builtin::Uint32) {
      return "uint32";
    }
    if (*builtin == Builtin::Uint64) {
      return "uint64";
    }
    if (*builtin == Builtin::String) {
      return "string";
    }
  }
  return *user;
}

bool Type::is_builtin() const {
  return builtin.has_value();
}

const Struct* Schema::find_struct(std::string_view n) const {
  auto it = structs.find(std::string(n));
  return it == structs.end() ? nullptr : &it->second;
}

const Function* Schema::find_function(std::string_view n) const {
  auto it = functions.find(std::string(n));
  return it == functions.end() ? nullptr : &it->second;
}
} // namespace ct
