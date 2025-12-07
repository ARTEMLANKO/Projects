#include "serializer.h"

#include "endian.h"
#include "my_types.h"
#include "request_classes.h"

#include <xxhash.h>

namespace ct {
// сериализация примитивных типов
void Serializer::serialize_builtin(Builtin b, const Value& v) {
  if (b == Builtin::String) {
    if (!v.is<std::string>()) {
      throw SerializeError("excepted string");
    }
    const std::string& s = v.as<std::string>();
    put_be<uint32_t>(out, static_cast<uint32_t>(s.size()));
    put_bytes(out, std::as_bytes(std::span(s)));
  } else if (b == Builtin::Int32) {
    int64_t x;
    if (v.is_int()) {
      x = v.as_int();
    } else {
      throw SerializeError("excepted int32");
    }
    if (x > INT32_MAX) {
      throw SerializeError("int32 overflow");
    }
    if (x < INT32_MIN) {
      throw SerializeError("int32 underflow");
    }
    put_be<int32_t>(out, static_cast<int32_t>(x));
  } else if (b == Builtin::Int64) {
    int64_t x;
    if (v.is_int()) {
      x = v.as_int();
    } else {
      throw SerializeError("excepted int64");
    }
    put_be<int64_t>(out, x);
  } else if (b == Builtin::Uint32) {
    uint64_t x;
    if (v.is_int()) {
      if (v.as_int() < 0) {
        throw SerializeError("negative for uint32");
      }
      x = static_cast<uint64_t>(v.as_int());
    } else if (v.is<uint64_t>()) {
      x = v.as<uint64_t>();
    } else {
      throw SerializeError("excepted uint32");
    }
    if (x > 0xffffffffULL) {
      throw SerializeError("uint32 out of range");
    }
    put_be<uint32_t>(out, static_cast<uint32_t>(x));
  } else if (b == Builtin::Uint64) {
    uint64_t x;
    if (v.is_int()) {
      if (v.as_int() < 0) {
        throw SerializeError("negative for uint64");
      }
      x = static_cast<uint64_t>(v.as_int());
    } else if (v.is<uint64_t>()) {
      x = v.as<uint64_t>();
    } else {
      throw SerializeError("excepted uint64");
    }
    put_be<uint64_t>(out, x);
  }
}

// сериализация всех полей
void Serializer::serialize_struct(const Struct& st, const StructValue& sv) {
  for (auto& fld : st.fields) {
    auto it = sv.fields.find(fld.name);
    if (it == sv.fields.end()) {
      throw SerializeError("missing struct field '" + fld.name + "' for '" + st.name + "'");
    }
    serialize_value(fld.type, it->second);
  }
}

// is_builtin - проверка, встроенный ли тип
void Serializer::serialize_value(const Type& t, const Value& v) {
  if (t.is_builtin()) {
    serialize_builtin(*t.builtin, v);
  } else {
    const auto* st = sch.find_struct(*t.user);
    if (!st) {
      throw SerializeError("unknown struct type");
    }
    const auto& sv = v.as<StructValue>();
    if (!sv.struct_name.empty() && sv.struct_name != st->name) {
      throw SerializeError("struct literal name mismatch");
    }
    serialize_struct(*st, sv);
  }
}

// сначала сериализуем название функции, потом её аргументы
std::vector<std::byte> serialize_call(const Schema& sch, const Call& call) {
  const auto* fn = sch.find_function(call.func_name);
  if (!fn) {
    throw SerializeError("unknown function '" + call.func_name + "'");
  }
  std::unordered_map<std::string, Value> provided;
  for (auto& a : call.args) {
    provided.emplace(a.name, a.value);
  }
  Serializer ser(sch);
  uint32_t h = XXH32(call.func_name.data(), call.func_name.size(), 0);
  put_be<uint32_t>(ser.out, h);
  for (auto& def : fn->args) {
    auto it = provided.find(def.name);
    if (it == provided.end()) {
      throw SerializeError("missing arg");
    }
    ser.serialize_value(def.type, it->second);
  }
  return ser.out;
}
} // namespace ct
