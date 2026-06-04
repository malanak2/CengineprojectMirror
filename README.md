# C Engine Project

### Setup
---
#### Install following dependencies:
* [CMake](https://cmake.org/)
* [GLAD](https://glad.dav1d.de/)
  * Profile core
  * gl version 4.6
* [GLFW](https://github.com/glfw/glfw)
* [GLM](https://github.com/g-truc/glm)
* [stb_image](https://github.com/nothings/stb)
* [JSON](https://github.com/nlohmann/json)
* [Dear ImGui](https://github.com/ocornut/imgui)
* [ImPlot](https://github.com/epezent/implot)
* [spdlog](https://github.com/gabime/spdlog)

### Building
---
#### Clone the repository
```bash
git clone https://git.malanak.cz/tmal/cengineproject.git
cd cengineproject
```
#### Generate cmake files
```bash
cmake -S . -B out
```

#### Build the project
```bash
cd out
make
```
