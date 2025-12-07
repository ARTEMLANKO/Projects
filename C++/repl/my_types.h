#pragma once
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace ct {

// типы после парсинга схемы

enum class Builtin {
  Int32,
  Int64,
  Uint32,
  Uint64,
  String
};

// это класс обёртка над каждым типом в подаваемой схеме. по сути в нём хранится название тип
// всегда будет активным только 1 optional (в комментах в пулл реквесте пояснил за это)
struct Type {
  std::optional<Builtin> builtin;
  std::optional<std::string> user;

  static Type builtin_of(Builtin b);

  static Type user_of(std::string_view b);
  std::string str() const;

  bool is_builtin() const;
};

struct Field {
  std::string name;
  Type type;
};

struct Struct {
  std::string name;
  std::vector<Field> fields;
};

struct Arg {
  std::string name;
  Type type;
};

struct Function {
  std::string name;
  Type return_type;
  std::vector<Arg> args;
};

struct Schema {
  std::unordered_map<std::string, Struct> structs;
  std::unordered_map<std::string, Function> functions;

  const Struct* find_struct(std::string_view n) const;
  const Function* find_function(std::string_view n) const;
};

struct SchemaError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

} // namespace ct
