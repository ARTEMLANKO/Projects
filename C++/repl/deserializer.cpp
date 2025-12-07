#include "deserializer.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace ct {
uint8_t Cursor::get8() {
  if (i >= n) {
    throw DeserError("EOF");
  }
  return std::to_integer<uint8_t>(p[i++]);
}

std::string Cursor::get_string() {
  uint32_t len = get_be<uint32_t>();
  std::string s;
  for (uint32_t k = 0; k < len; k++) {
    s += char(get8());
  }
  return s;
}

void read_struct(Cursor& c, const Schema& sch, const Struct& st, std::string& out) {
  out += st.name + "{";
  bool first = true;
  for (auto& f : st.fields) {
    if (!first) {
      out += ", ";
    }
    out += f.name + "=";
    read_value(c, sch, f.type, out);
    first = false;
  }
  out += "}";
}

void read_value(Cursor& c, const Schema& sch, const Type& t, std::string& out) {
  if (t.is_builtin()) {
    if (*t.builtin == Builtin::String) {
      auto s = c.get_string();
      out += "\"" + s + "\"";
    } else if (*t.builtin == Builtin::Int32) {
      out += std::to_string(c.get_be<int32_t>());
    } else if (*t.builtin == Builtin::Int64) {
      out += std::to_string(c.get_be<int64_t>());
    } else if (*t.builtin == Builtin::Uint32) {
      out += std::to_string(c.get_be<uint32_t>());
    } else if (*t.builtin == Builtin::Uint64) {
      out += std::to_string(c.get_be<uint64_t>());
    }
  } else {
    auto st = sch.find_struct(*t.user);
    if (!st) {
      throw DeserError("unknown struct type");
    }
    read_struct(c, sch, *st, out);
  }
}

std::string deserialize_response_to_string(const Schema& sch, const Function& fn, std::span<const std::byte> bytes) {
  Cursor cur{bytes.data(), bytes.size()};
  std::string out;
  read_value(cur, sch, fn.return_type, out);
  if (cur.i != cur.n) {
    throw DeserError("extra bytes after response value");
  }
  return out;
}
} // namespace ct
