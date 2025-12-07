#include "autocomplete.h"

#include "my_types.h"
#include "request_parser.h"

#include <cctype>
#include <map>
#include <set>
#include <vector>

namespace ct {

void skip_spaces(TextView& txt) {
  while (!txt.eof() && std::isspace(txt.peek())) {
    txt.advance();
  }
}

std::optional<std::string> get_func(TextView& txt, const std::vector<std::string>& funcs, std::string& out) {
  skip_spaces(txt);
  auto start = txt.it;
  while (!txt.eof() && txt.peek() != '(') {
    txt.advance();
  }
  std::string token = std::string(txt.sv.substr(start, txt.it - start));
  for (auto& f : funcs) {
    if (f.rfind(token, 0) == 0) {
      out += f.substr(token.size());
      return f;
    }
  }
  return std::nullopt;
}

std::optional<std::string> get_arg(TextView& txt, std::set<std::string>& args, std::string& out) {
  skip_spaces(txt);
  auto start = txt.it;
  while (!txt.eof() && txt.peek() != '=') {
    txt.advance();
  }
  std::string token = std::string(txt.sv.substr(start, txt.it - start));
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (it->rfind(token, 0) == 0) {
      out += it->substr(token.size());
      std::string chosen = *it;
      args.erase(it);
      return chosen;
    }
  }
  return std::nullopt;
}

std::string collect_until_any(TextView& txt, std::string_view stops) {
  size_t start = txt.it;
  while (!txt.eof() && stops.find(txt.peek()) == std::string_view::npos) {
    txt.advance();
  }
  return std::string(txt.sv.substr(start, txt.it - start));
}

bool append_if_missing(TextView& txt, char ch, std::string& out) {
  if (!txt.eof() && txt.peek() == ch) {
    txt.advance();
    return true;
  }
  out += ch;
  return false;
}

ParseStruct fill_structure(std::string& out, std::string_view type, TextView& txt, const Schema& sch);

ParseStruct fill_primitive(std::string_view type, TextView& txt, std::string& out, const Schema& sch) {
  if (type == "string") {
    if (append_if_missing(txt, '\"', out)) {
      collect_until_any(txt, "\"");
      if (!txt.eof()) {
        txt.advance();
        return ParseStruct::Finished;
      }
      return ParseStruct::Incomplete;
    }
    return ParseStruct::Incomplete;
  }
  std::set<std::string> primitives;
  primitives.insert("int32");
  primitives.insert("uint32");
  primitives.insert("int64");
  primitives.insert("uint64");
  if (primitives.find(std::string(type)) != primitives.end()) {
    auto before = txt.it;
    collect_until_any(txt, "}),");
    return (!txt.eof() && txt.it != before) ? ParseStruct::Finished : ParseStruct::Incomplete;
  }
  return fill_structure(out, type, txt, sch);
}

ParseStruct fill_structure(std::string& out, std::string_view type, TextView& txt, const Schema& sch) {
  if (!txt.eof() && txt.peek() != '{') {
    std::string prefix = collect_until_any(txt, "{");
    if (type.rfind(prefix, 0) == 0) {
      out += type.substr(prefix.size());
    }
    append_if_missing(txt, '{', out);
  } else if (txt.eof()) {
    append_if_missing(txt, '{', out);
  } else {
    txt.advance();
  }

  const Struct* st = sch.find_struct(type);
  if (st->fields.empty()) {
    append_if_missing(txt, '}', out);
    return ParseStruct::Finished;
  }

  std::set<std::string> remaining;
  std::map<std::string, std::string> mapping;
  for (auto& f : st->fields) {
    remaining.insert(f.name);
    mapping[f.name] = f.type.str();
  }
  while (!remaining.empty()) {
    auto chosen = *get_arg(txt, remaining, out);
    append_if_missing(txt, '=', out);
    ParseStruct stt = fill_primitive(mapping[chosen], txt, out, sch);
    if (stt == ParseStruct::Finished) {
      if (remaining.empty()) {
        append_if_missing(txt, '}', out);
        return ParseStruct::Finished;
      } else {
        append_if_missing(txt, ',', out);
        append_if_missing(txt, ' ', out);
        continue;
      }
    }
    if (txt.eof()) {
      break;
    }
  }
  return ParseStruct::Incomplete;
}

void parse_function(std::string& out, std::string_view fnName, TextView& txt, const Schema& sch) {
  append_if_missing(txt, '(', out);

  const Function* fn = sch.find_function(fnName);
  if (fn->args.empty()) {
    append_if_missing(txt, ')', out);
    return;
  }

  std::set<std::string> remaining;
  std::map<std::string, std::string> mapping;
  for (auto& f : fn->args) {
    remaining.insert(f.name);
    mapping[f.name] = f.type.str();
  }
  while (!remaining.empty()) {
    std::string chosen = *get_arg(txt, remaining, out);
    append_if_missing(txt, '=', out);
    ParseStruct stt = fill_primitive(mapping[chosen], txt, out, sch);
    if (stt == ParseStruct::Finished) {
      if (remaining.empty()) {
        append_if_missing(txt, ')', out);
        return;
      } else {
        append_if_missing(txt, ',', out);
        append_if_missing(txt, ' ', out);
        continue;
      }
    }
    if (txt.eof()) {
      break;
    }
  }
}

std::string autocomplete(std::string_view input, const Schema& sch) {
  TextView txt(input);
  std::string suffix;
  std::vector<std::string> funcs;
  for (auto& [name, _] : sch.functions) {
    funcs.push_back(name);
  }
  auto fnName = get_func(txt, funcs, suffix);
  if (!fnName) {
    return std::string(input);
  }
  parse_function(suffix, *fnName, txt, sch);
  return std::string(input) + suffix;
}
} // namespace ct
