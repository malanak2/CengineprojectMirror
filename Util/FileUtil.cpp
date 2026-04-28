#include "FileUtil.hpp"
#include "fstream"
#include <iterator>
#include <regex>

/// Loads a text file from the resources directory
int FileUtil::ReadFile(std::string path, std::string *result) {
  std::ifstream file;
  path = "resources/" + path;
  // Everything should be in the resources directory. if it is not, then move it
  // there!
  path = std::regex_replace(path, std::regex("\\.\\."), ".");
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

int FileUtil::SaveFile(std::string path, std::string *content) {
  // TODO
  return -1;
}

/// Loads a binary file from the resources directory
int FileUtil::LoadBinary(std::string path, std::vector<unsigned char> *res) {
  res->clear();
  std::ifstream file;
  path = "resources/" + path;
  // Everything should be in the resources directory. if it is not, then move it
  // there!
  path = std::regex_replace(path, std::regex("\\.\\."), ".");
  file.open(path, std::ios::binary);
  if (!file.is_open()) {
    return -1;
  }
  std::vector<unsigned char> out(std::istreambuf_iterator<char>(file), {});
  res->assign(out.begin(), out.end());
  file.close();
  return 0;
}
