#pragma once

#include <string>
namespace Editor {
class Window {
public:
  void Open() { isOpen = true; }
  void Close() { isOpen = false; }
  void Draw() {
    if (isOpen)
      draw();
  }
  virtual std::string GetName() = 0;

  bool isOpen = true;

protected:
  virtual void draw() = 0;
};

} // namespace Editor
