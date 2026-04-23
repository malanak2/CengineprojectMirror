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

void sigabrtHandler(int sig) { sigsegvHandler(sig); }

int main() {
  signal(SIGSEGV, sigsegvHandler);
  signal(SIGABRT, sigabrtHandler);
  CPPTRACE_TRY {
    auto engine = new Engine();
    if (engine->Init() != 0) {
      throw std::runtime_error("Failed to initialize engine");
    }
    engine->Run();
  }
  CPPTRACE_CATCH(const std::exception &e) {
    cpptrace::from_current_exception().print();
    return -1;
  }
  return 0;
}
