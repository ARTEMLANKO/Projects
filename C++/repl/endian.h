#pragma once
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace ct {
template <typename T>
void put_be(std::vector<std::byte>& out, T v) {
  using U = std::make_unsigned_t<T>;
  U uv = static_cast<U>(v);
  for (int s = (sizeof(T) - 1) * 8; s >= 0; s -= 8) {
    out.push_back(std::byte((uv >> s) & 0xff));
  }
}

void put_bytes(std::vector<std::byte>& out, std::span<const std::byte> bytes);

} // namespace ct
