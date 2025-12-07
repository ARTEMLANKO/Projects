#pragma once
#include "endian.h"
#include "my_types.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>

namespace ct {
struct DeserError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct Cursor {
  const std::byte* p;
  std::size_t n;
  std::size_t i = 0;

  uint8_t get8();

  template <typename T>
  T get_be() {
    T v = 0;
    for (size_t k = 0; k < sizeof(T); k++) {
      v = (v << 8) | get8();
    }
    return v;
  }

  std::string get_string();
};

void read_value(Cursor& c, const Schema& sch, const Type& t, std::string& out);

void read_struct(Cursor& c, const Schema& sch, const Struct& st, std::string& out);

std::string deserialize_response_to_string(const Schema& sch, const Function& fn, std::span<const std::byte> bytes);
} // namespace ct
