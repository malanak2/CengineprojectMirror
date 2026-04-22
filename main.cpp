#include <cpptrace/basic.hpp>
#include <csignal>
#include <exception>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "Engine/Engine.hpp"
#include "cpptrace/from_current.hpp"

void sigsegvHandler(int sig) {
  cpptrace::generate_trace().print();
  exit(sig);
}

int main() {
  signal(SIGSEGV, sigsegvHandler);
  signal(SIGABRT, sigsegvHandler);
  CPPTRACE_TRY {
    auto engine = new Engine();
    if (engine->Init() != 0) {
      return -1;
    }
    engine->Run();
    return 0;
  }
  CPPTRACE_CATCH(const std::exception &e) {
    cpptrace::from_current_exception().print();
  }
}
