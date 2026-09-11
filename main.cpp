#include <cpptrace/basic.hpp>
#include <csignal>
#include <exception>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "Engine/Engine.hpp"
#include "ScriptSystem.hpp"
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
    /// First init engine
    Engine::Engine::Init();
    /// Register components
    // REGISTER_SCRIPT(script);
    /// Load scene
    Engine::Engine::LoadScene();
    /// Run
    Engine::Engine::instance->Run();
  }
  CPPTRACE_CATCH(const std::exception &e) {
    cpptrace::from_current_exception().print();
    return -1;
  }
  return 0;
}
