#include <csignal>
#include <exception>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "Engine/Engine.hpp"

void sigsegvHandler(int sig) {
  exit(sig);
}

void sigabrtHandler(int sig) { sigsegvHandler(sig); }

int main() {
  signal(SIGSEGV, sigsegvHandler);
  signal(SIGABRT, sigabrtHandler);
    auto engine = new Engine();
    if (engine->Init() != 0) {
      return -1;
    }
    engine->Run();
    return 0;
  }
