#include "repl.h"

#include "autocomplete.h"
#include "deserializer.h"
#include "request_parser.h"
#include "rpc/client.h"
#include "schema_loader.h"
#include "serializer.h"

#include <iostream>
#include <replxx.hxx>
#include <stdexcept>
#include <string>
#include <vector>

namespace ct {

void run_no_tty(const Schema& sch, ct::rpc::Client& client) {
  std::string line;
  while (std::getline(std::cin, line)) {
    try {
      auto call = RequestParser::parse(line);
      auto req = serialize_call(sch, call);
      auto resp_bytes = client.send(req);
      const auto* fn = sch.find_function(call.func_name);
      std::string out = deserialize_response_to_string(sch, *fn, resp_bytes);
      std::cout << out << '\n';
    } catch (std::runtime_error& e) {
      std::cout << "Error: " << e.what() << '\n';
    }
  }
  std::cout << "Goodbye!" << '\n';
}

void run_tty(const Schema& sch, ct::rpc::Client& client) {
  replxx::Replxx rx;
  rx.set_max_history_size(1000);
  rx.bind_key(replxx::Replxx::KEY::TAB, [&](char32_t) {
    auto state = rx.get_state();
    std::string input(state.text());
    std::string completed = autocomplete(input, sch);
    rx.set_state({completed.c_str(), static_cast<int32_t>(completed.size())});
    return replxx::Replxx::ACTION_RESULT::CONTINUE;
  });
  while (true) {
    const char* raw = rx.input(">> ");
    if (!raw) {
      std::cout << "Goodbye!" << '\n';
      break;
    }
    std::string line(raw);
    if (line.empty()) {
      continue;
    }
    rx.history_add(line.c_str());
    if (line == "exit") {
      std::cout << "Goodbye!" << '\n';
      break;
    }
    try {
      auto call = RequestParser::parse(line);
      auto req = serialize_call(sch, call);
      auto resp_bytes = client.send(req);
      const auto* fn = sch.find_function(call.func_name);
      if (!fn) {
        throw std::runtime_error("Unknown function");
      }
      std::string out = deserialize_response_to_string(sch, *fn, resp_bytes);
      std::cout << out << '\n';
    } catch (const std::runtime_error& e) {
      std::cout << "Error: " << e.what() << '\n';
    }
  }
}

void run(const Options& opts) {
  Schema schema;
  try {
    schema = load_schema_file(opts.schema_path);
  } catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << '\n';
    std::exit(1);
  }

  rpc::Client client(opts.rpc_host, opts.rpc_port, opts.rpc_path);
  if (opts.no_tty) {
    run_no_tty(schema, client);
  } else {
    run_tty(schema, client);
  }
}
} // namespace ct
