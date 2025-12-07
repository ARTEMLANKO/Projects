#include "request_classes.h"

#include <limits>
#include <string>
#include <variant>

namespace ct {

bool Value::is_int() const {
  if (is<uint64_t>()) {
    uint64_t v = as<uint64_t>();
    if (v <= std::numeric_limits<Int>::max()) {
      return true;
    }
  }
  return std::holds_alternative<Int>(*this);
}

Int Value::as_int() const {
  if (is<uint64_t>()) {
    uint64_t vvv = as<uint64_t>();
    long long ll = vvv;
    return ll;
  }
  return std::get<Int>(*this);
}

} // namespace ct
