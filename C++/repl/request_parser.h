#pragma once
#include "request_classes.h"

#include <cctype>
#include <stdexcept>
#include <string>

namespace ct {

struct ParseError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

// класс-обёртка над строкой-запросом
class Lexer {
  std::string src;
  std::size_t i = 0;

public:
  Lexer(std::string s);

  bool eof() const;

  char peek() const;

  char get();

  void skip_ws();

  std::string ident();

  std::string string_lit();

  std::variant<int64_t, uint64_t> integer();

  bool consume(char c);

  void except(char c);
};

class RequestParser {
public:
  static Call parse(std::string s);

private:
  static Value parse_value(Lexer& lx);
};
} // namespace ct
