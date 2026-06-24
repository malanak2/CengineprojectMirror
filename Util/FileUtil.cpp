#include "FileUtil.hpp"
#include "fstream"
#include <fstream>
#include <iterator>
#include <memory>
#include <regex>
#define STB_IMAGE_IMPLEMENTATION
#include "External/stb/stb_image.h"
/// Loads a text file from the resources directory
int FileUtil::ReadFile(std::string path, std::string *result) {
  std::ifstream file;
  path = sanitizePath(path);
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
  std::ofstream file;
  path = sanitizePath(path);
  file.open(path, std::ios::out);
  if (!file.is_open()) {
    return -1;
  }
  file << *content;
  file.close();

  return 0;
}

/// Loads a binary file from the resources directory
int FileUtil::LoadBinary(std::string path, std::vector<unsigned char> *res) {
  res->clear();
  std::ifstream file;
  path = sanitizePath(path);
  file.open(path, std::ios::binary);
  if (!file.is_open()) {
    return -1;
  }
  std::vector<unsigned char> out(std::istreambuf_iterator<char>(file), {});
  res->assign(out.begin(), out.end());
  file.close();
  return 0;
}
std::shared_ptr<FileUtil::ImageFile> FileUtil::LoadImage(std::string path) {
  path = sanitizePath(path);
  auto ret = std::make_shared<ImageFile>();
  ret->data =
      stbi_load(path.c_str(), &ret->width, &ret->height, &ret->nrChannels, 0);
  return ret;
}

std::string FileUtil::sanitizePath(std::string path) {
  stbi_set_flip_vertically_on_load(true);
  path = "resources/" + path;
  // Everything should be in the resources directory. if it is not, then move it
  // there!
  path = std::regex_replace(path, std::regex("\\.\\."), ".");
  return path;
}
FileUtil::ImageFile::~ImageFile() {
  if (data) {
    stbi_image_free(data);
  }
}
