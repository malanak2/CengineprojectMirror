#include <iostream>

#include "Engine/Engine.hpp"


int main() {
    auto engine = new Engine();
    engine->Init();
    engine->Run();
    return 0;
}
