#include "FileUtil.hpp"
#include "fstream"
#include <fstream>
#include <iterator>
#include <memory>
#include <regex>
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "External/stb/stb_image.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC pop_options
#endif
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
  stbi_set_flip_vertically_on_load(1);
  int comp = 0;
  if (stbi_info(path.c_str(), &ret->width, &ret->height, &comp)) {
    int desired = (comp == 1) ? 1 : 4;
    ret->data = stbi_load(path.c_str(), &ret->width, &ret->height,
                          &ret->nrChannels, desired);
    if (ret->data) {
      ret->nrChannels = desired;
    }
  } else {
    ret->data =
        stbi_load(path.c_str(), &ret->width, &ret->height, &ret->nrChannels, 0);
  }
  return ret;
}
std::string FileUtil::sanitizePath(std::string path) {
  std::replace(path.begin(), path.end(), '\\', '/');

  std::vector<std::string> parts;
  size_t pos = 0;
  while (pos <= path.size()) {
    size_t next = path.find('/', pos);
    if (next == std::string::npos)
      next = path.size();
    std::string_view seg(path.data() + pos, next - pos);
    if (seg.empty() || seg == ".") {
    } else if (seg == "..") {
      if (!parts.empty())
        parts.pop_back();
    } else {
      parts.emplace_back(seg);
    }
    pos = next + 1;
  }

  // accept both "textures/a.png" and "resources/textures/a.png"
  size_t first = (!parts.empty() && parts.front() == "resources") ? 1 : 0;
  std::string result = "resources";
  for (size_t i = first; i < parts.size(); ++i) {
    result += '/';
    result += parts[i];
  }
  return result;
}
FileUtil::ImageFile::~ImageFile() {
  if (data) {
    stbi_image_free(data);
  }
}
