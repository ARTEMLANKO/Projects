#include "request_parser.h"

#include <cctype>
#include <climits>
#include <limits>
#include <string>
#include <unordered_map>

namespace ct {

Lexer::Lexer(std::string s)
    : src(s) {}

bool Lexer::eof() const {
  return i >= src.size();
}

char Lexer::peek() const {
  return eof() ? '\0' : src[i];
}

char Lexer::get() {
  return eof() ? '\0' : src[i++];
}

void Lexer::skip_ws() {
  while (!eof() && std::isspace(peek())) {
    ++i;
  }
}

// парсим слово
std::string Lexer::ident() {
  skip_ws();
  if (!(std::isalpha(peek()) || peek() == '_')) {
    throw ParseError("Error: identifier excepted");
  }
  std::string out;
  while (!eof() && (std::isalnum(peek()) || peek() == '_')) {
    out += get();
  }
  return out;
}

// парсим строковый литерал

std::string Lexer::string_lit() {
  skip_ws();
  if (peek() != '"') {
    throw ParseError("string literal excepted");
  }
  get();
  std::string out;
  while (!eof() && peek() != '"') {
    char c = get();
    if (c == '\\') {
      if (eof()) {
        throw ParseError("bad escape");
      }
      char e = get();
      if (e == 'n') {
        out += '\n';
      } else if (e == 't') {
        out += '\t';
      } else if (e == '\\') {
        out += '\\';
      } else if (e == '"') {
        out += '"';
      } else {
        throw ParseError(std::string("unknown escape \\") + e);
      }
    } else {
      out += c;
    }
  }
  get(); // пропускаем вторую "
  if (eof()) {
    throw ParseError("Error: unterminal string");
  }
  return out;
}

// возвращаем variant. Если значение не влезет в int64, но влезет в uint64, то храним значение во втором поле variant
std::variant<int64_t, uint64_t> Lexer::integer() {
  skip_ws();
  bool neg = false;
  if (peek() == '+' || peek() == '-') {
    neg = (get() == '-');
  }
  if (!std::isdigit(peek())) {
    throw ParseError("Error: integer excepted");
  }
  uint64_t acc = 0;
  while (!eof() && std::isdigit(peek())) {
    int d = get() - '0';
    if (acc > (ULLONG_MAX - d) / 10) {
      throw ParseError("ULL overflow");
    }
    acc *= 10;
    acc += d;
  }
  if (neg) {
    if (acc > LLONG_MAX + 1ULL) {
      throw ParseError("LL underflow");
    }
    if (acc == static_cast<uint64_t>(9223372036854775807) + 1ULL) {
      return std::numeric_limits<int64_t>::min();
    }
    return -1 * static_cast<int64_t>(acc);
  }
  return acc;
}

bool Lexer::consume(char c) {
  skip_ws();
  if (peek() == c) {
    get();
    return true;
  }
  return false;
}

void Lexer::except(char c) {
  if (!consume(c)) {
    throw ParseError("excepted '" + std::to_string(c) + "'");
  }
}

Call RequestParser::parse(std::string s) {
  Lexer lx(s);
  Call call;
  call.func_name = lx.ident();
  lx.except('(');
  lx.skip_ws();
  if (!lx.consume(')')) {
    for (;;) {
      std::string argname = lx.ident();
      lx.except('=');
      Value val = parse_value(lx);
      call.args.push_back({argname, val});
      lx.skip_ws();
      if (lx.consume(')')) {
        break;
      }
      lx.except(',');
      lx.skip_ws();
      lx.skip_ws();
    }
  }
  lx.skip_ws();
  if (!lx.eof()) {
    throw ParseError("trailing characters after ')'");
  }
  return call;
}

Value RequestParser::parse_value(Lexer& lx) {
  lx.skip_ws();
  char c = lx.peek();
  if (c == '"') {
    return Value{lx.string_lit()};
  }
  if (c == '{' || std::isalpha(c) || c == '_') {
    std::string maybeName;
    if (c != '{') {
      maybeName = lx.ident();
      lx.skip_ws();
      if (!lx.consume('{')) {
        throw ParseError("excepted '{' to start struct literal");
      }
    } else {
      lx.get();
    }
    StructValue sv;
    sv.struct_name = maybeName;
    lx.skip_ws();
    if (!lx.consume('}')) {
      for (;;) {
        std::string fname = lx.ident();
        lx.except('=');
        Value fval = parse_value(lx);
        if (!sv.fields.emplace(fname, fval).second) {
          throw ParseError("duplicate field in struct");
        }
        lx.skip_ws();
        if (lx.consume('}')) {
          break;
        }
        lx.except(',');
        lx.skip_ws();
      }
    }
    return Value(sv);
  }
  std::variant<int64_t, uint64_t> val = lx.integer();
  if (std::holds_alternative<int64_t>(val)) {
    int64_t v = std::get<int64_t>(val);
    return Value(v);
  }
  uint64_t v = std::get<uint64_t>(val);
  return Value(v);
}
} // namespace ct
