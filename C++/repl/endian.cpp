#include "endian.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace ct {

void put_bytes(std::vector<std::byte>& out, std::span<const std::byte> bytes) {
  out.insert(out.end(), bytes.begin(), bytes.end());
}
} // namespace ct
