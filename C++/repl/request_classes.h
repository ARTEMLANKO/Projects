#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ct {

// типы после парсинга запроса
struct Value;

using Int = int64_t;
using UInt = uint64_t;

struct StructValue {
  std::string struct_name;
  std::unordered_map<std::string, Value> fields;
};

struct Value : std::variant<std::string, Int, UInt, StructValue> {
  using std::variant<std::string, Int, UInt, StructValue>::variant;

  template <typename T>
  bool is() const {
    return std::holds_alternative<T>(*this);
  }

  template <typename T>
  T& as() {
    return std::get<T>(*this);
  }

  template <typename T>
  const T& as() const {
    return std::get<T>(*this);
  }

  bool is_int() const;

  Int as_int() const;
};

struct NamedArg {
  std::string name;
  Value value;
};

struct Call {
  std::string func_name;
  std::vector<NamedArg> args;
};
} // namespace ct
