# C Engine Project

### Setup
---
#### Install following dependencies:
* [CMake](https://cmake.org/)
* [GLAD](https://glad.dav1d.de/)
  * Profile core
  * gl version 4.6
  * Put into Engine/External/glad
* [GLFW](https://github.com/glfw/glfw)
* [GLM](https://github.com/g-truc/glm)
* [stb_image](https://github.com/nothings/stb)
  * Put into Engine/Util/External/stb/stb_image.h
* [JSON](https://github.com/nlohmann/json)
* [Dear ImGui](https://github.com/ocornut/imgui)
* [ImPlot](https://github.com/epezent/implot)
* [spdlog](https://github.com/gabime/spdlog)
* [ImGui Test Engine](https://github.com/ocornut/imgui_test_engine)
  * Clone into Editor/External
#### Automatically installed
* [Tracy](https://github.com/wolfpld/tracy)[Engine](Engine)

### Building
---
#### Clone the repository
```bash
git clone https://git.malanak.cz/tmal/cengineproject.git
cd cengineproject
```
#### Generate cmake files
```bash
cmake -G Ninja -S . -B out
```

#### Build the project
```bash
cd out
ninja
```

#### Generate shaders
##### Windows
```powershell
scripts/generate_shaders.ps1
```

##### Linux
```bash
scripts/generate_shaders.sh
```

#### Setup symlink out/.../bin/resources pointing to resources

### Profiling
---
The program uses [Tracy](https://github.com/wolfpld/tracy) for profiling

### Debugging
---
The program uses [ImGui Test Engine](https://github.com/ocornut/imgui_test_engine)