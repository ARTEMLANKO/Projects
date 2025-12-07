#pragma once
#include "deserializer.h"
#include "rpc/client.h"

#include <string>

namespace ct {

struct Options {
  std::string schema_path;
  bool no_tty = false;
  std::string rpc_host = "127.0.0.1";
  int rpc_port = 8080;
  std::string rpc_path;
};

void run_no_tty(const Schema& sch, ct::rpc::Client& client);

void run_tty(const Schema& sch, ct::rpc::Client& client);

void run(const Options& opts);
} // namespace ct
