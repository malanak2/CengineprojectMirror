#include "FileUtil.hpp"
#include "fstream"

int FileUtil::ReadFile(std::string path, std::string *result) {
  std::ifstream file;
  file.open(path, std::ios::in | std::ios::ate);
  if (!file.is_open()) {
    return -1;
  }
  auto size = file.tellg();
  file.seekg(0);
  std::string out(size, '\0');
  file.read(&out[0], size);
  file.close();
  result->append(out);
  return 0;
}
