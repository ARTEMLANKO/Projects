#pragma once
#include "endian.h"
#include "my_types.h"
#include "request_classes.h"

#include <stdexcept>

namespace ct {
struct SerializeError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct Serializer {
  const Schema& sch;
  std::vector<std::byte> out;

  Serializer(const Schema& s)
      : sch(s) {}

  void serialize_builtin(Builtin b, const Value& v);

  void serialize_struct(const Struct& st, const StructValue& sv);

  void serialize_value(const Type& t, const Value& v);

  std::vector<std::byte> serialize_call(const Call& call);
};

std::vector<std::byte> serialize_call(const Schema& sch, const Call& call);
} // namespace ct
