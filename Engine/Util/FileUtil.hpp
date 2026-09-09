//
// Created by malan on 21.04.2026.
//

#pragma once
#include <memory>
#include <string>
#include <vector>
class FileUtil {
public:
  class ImageFile {
  public:
    int width, height, nrChannels;
    unsigned char *data;
    ~ImageFile();
  };
  static int ReadFile(std::string path, std::string *result);
  static int SaveFile(std::string path, std::string *content);
  static int LoadBinary(std::string path, std::vector<unsigned char> *res);
  static std::shared_ptr<ImageFile> LoadImage(std::string path);

private:
  static std::string sanitizePath(std::string path);
};
