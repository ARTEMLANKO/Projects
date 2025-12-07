#include "schema_parser.h"

#include "ctpg/ctpg.hpp"

#include <iostream>
#include <unordered_set>

namespace ct {

constexpr ctpg::string_term t_fn("fn");
constexpr ctpg::string_term t_struct("struct");
constexpr ctpg::string_term t_arrow("->");
constexpr ctpg::char_term t_lbrace('{');
constexpr ctpg::char_term t_rbrace('}');
constexpr ctpg::char_term t_sc(';');

constexpr ctpg::string_term t_i32("int32");
constexpr ctpg::string_term t_i64("int64");
constexpr ctpg::string_term t_u32("uint32");
constexpr ctpg::string_term t_u64("uint64");
constexpr ctpg::string_term t_str("string");

constexpr char ident_pat[] = "[_A-Za-z][_A-Za-z0-9]*";
constexpr ctpg::regex_term<ident_pat> t_ident("ident");

static constexpr auto N_SCHEMA = ctpg::nterm<Schema>("schema");
static constexpr auto N_ITEMS = ctpg::nterm<Schema>("items");
static constexpr auto N_ITEM = ctpg::nterm<Schema>("item");

static constexpr auto N_STRUCT = ctpg::nterm<Struct>("struct");
static constexpr auto N_SFIELDS = ctpg::nterm<std::vector<Field>>("sfields");
static constexpr auto N_SFIELD = ctpg::nterm<Field>("sfield");

static constexpr auto N_FN = ctpg::nterm<Function>("fn");
static constexpr auto N_FARGS = ctpg::nterm<std::vector<Arg>>("fargs");
static constexpr auto N_FARG = ctpg::nterm<Arg>("farg");

static constexpr auto N_TYPE = ctpg::nterm<Type>("type");

void ensure_unique(const std::vector<Field>& v, const std::string& ctx) {
  std::unordered_set<std::string> s;
  for (auto& f : v) {
    if (s.find(f.name) != s.end()) {
      throw SchemaError("Error: Duplicate field '" + f.name + "' in struct '" + ctx + "'");
    }
    s.insert(f.name);
  }
}

void ensure_unique(const std::vector<Arg>& v, const std::string& ctx) {
  std::unordered_set<std::string> s;
  for (auto& f : v) {
    if (s.find(f.name) != s.end()) {
      throw SchemaError("Error: Duplicate argument '" + f.name + "' in function '" + ctx + "'");
    }
    s.insert(f.name);
  }
}

Schema merge_items(Schema a, Schema b) {
  for (auto& s : a.structs) {
    if (b.structs.find(s.first) != b.structs.end()) {
      throw SchemaError("Error: Duplicate struct '" + s.first + "'");
    }
    b.structs.emplace(s.first, s.second);
  }
  for (auto& f : a.functions) {
    if (b.functions.find(f.first) != b.functions.end()) {
      throw SchemaError("Error: Duplicate function '" + f.first + "'");
    }
    b.functions.emplace(f.first, f.second);
  }
  return b;
}

Schema empty_items() {
  return Schema{};
}

Schema item_from_struct(Struct s) {
  Schema sch;
  sch.structs.emplace(s.name, s);
  return sch;
}

Schema item_from_fn(Function f) {
  Schema sch;
  sch.functions.emplace(f.name, f);
  return sch;
}

Struct make_struct(std::string_view, std::string_view id, char, std::vector<Field> fs, char) {
  Struct s{std::string(id), fs};
  ensure_unique(s.fields, s.name);
  return s;
}

std::vector<Field> append_field(std::vector<Field> rest, Field f) {
  rest.push_back(f);
  ensure_unique(rest, f.name);
  return rest;
}

std::vector<Field> empty_fields() {
  return {};
}

Field make_field(Type t, std::string_view id, char) {
  return Field{std::string(id), t};
}

Function
make_function(std::string_view, std::string_view id, std::string_view, Type ret, char, std::vector<Arg> args, char) {
  Function f{std::string(id), ret, args};
  ensure_unique(f.args, f.name);
  return f;
}

std::vector<Arg> append_arg(std::vector<Arg> rest, Arg a) {
  rest.push_back(a);
  ensure_unique(rest, a.name);
  return rest;
}

std::vector<Arg> empty_args() {
  return {};
}

Arg make_arg(Type t, std::string_view id, char) {
  Arg s{std::string(id), t};
  return s;
}

Type type_int32(std::string_view) {
  return Type::builtin_of(Builtin::Int32);
}

Type type_int64(std::string_view) {
  return Type::builtin_of(Builtin::Int64);
}

Type type_uint32(std::string_view) {
  return Type::builtin_of(Builtin::Uint32);
}

Type type_uint64(std::string_view) {
  return Type::builtin_of(Builtin::Uint64);
}

Type type_string(std::string_view) {
  return Type::builtin_of(Builtin::String);
}

Type type_user(std::string_view id) {
  return Type{std::nullopt, std::string(id)};
}

static constexpr auto SCHEMA_PARSER = ctpg::parser(
    N_SCHEMA,
    terms(t_fn, t_struct, t_arrow, t_lbrace, t_rbrace, t_sc, t_i32, t_i64, t_u64, t_u32, t_str, t_ident),
    nterms(N_SCHEMA, N_ITEMS, N_ITEM, N_STRUCT, N_SFIELDS, N_SFIELD, N_FN, N_FARGS, N_FARG, N_TYPE),
    rules(
        N_SCHEMA(N_ITEMS) >= [](Schema s) { return s; },

        N_ITEM(N_STRUCT) >= item_from_struct,
        N_ITEM(N_FN) >= item_from_fn,

        N_ITEMS(N_ITEM, N_ITEMS) >= merge_items,
        N_ITEMS() >= empty_items,

        N_STRUCT(t_struct, t_ident, t_lbrace, N_SFIELDS, t_rbrace) >= make_struct,

        N_SFIELD(N_TYPE, t_ident, t_sc) >= make_field,

        N_SFIELDS(N_SFIELDS, N_SFIELD) >= append_field,
        N_SFIELDS() >= empty_fields,

        N_FN(t_fn, t_ident, t_arrow, N_TYPE, t_lbrace, N_FARGS, t_rbrace) >= make_function,

        N_FARGS(N_FARGS, N_FARG) >= append_arg,
        N_FARGS() >= empty_args,

        N_FARG(N_TYPE, t_ident, t_sc) >= make_arg,

        N_TYPE(t_i32) >= type_int32,
        N_TYPE(t_i64) >= type_int64,
        N_TYPE(t_u32) >= type_uint32,
        N_TYPE(t_u64) >= type_uint64,
        N_TYPE(t_str) >= type_string,
        N_TYPE(t_ident) >= type_user
    )
);

void check_user_type(const Schema& out, const Type& t, const std::string& ctx) {
  if (!t.is_builtin() && !out.find_struct(*t.user)) {
    throw SchemaError("Error: Unknown type '" + *t.user + "' in " + ctx);
  }
}

Schema parse_schema_text(std::string_view text) {
  Schema out;
  using namespace ctpg::buffers;
  if (auto res = SCHEMA_PARSER.parse(string_buffer(std::string(text)))) {
    out = *res;
    for (auto& [_, s] : out.structs) {
      for (auto& f : s.fields) {
        check_user_type(out, f.type, "struct '" + s.name + "'");
        if (!f.type.is_builtin() && s.name == *f.type.user) {
          throw SchemaError("Recursive struct");
        }
      }
    }

    for (auto& [_, f] : out.functions) {
      check_user_type(out, f.return_type, "function return '" + f.name + "'");
      for (auto& a : f.args) {
        check_user_type(out, a.type, "function arg '" + f.name + "." + a.name + "'");
      }
    }
    return out;
  }
  throw SchemaError("Error: failed to parse schema");
}
} // namespace ct
