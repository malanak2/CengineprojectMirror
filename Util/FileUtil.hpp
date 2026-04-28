//
// Created by malan on 21.04.2026.
//

#pragma once
#include <string>
#include <vector>
#include "spdlog/spdlog.h"
class FileUtil {
public:
  static int ReadFile(std::string path, std::string *result);
  static int SaveFile(std::string path, std::string *content);
  static int LoadBinary(std::string path, std::vector<unsigned char> *res);
};
