//
// Created by malan on 21.04.2026.
//

#pragma once
#include <string>
#include <vector>
class FileUtil {
public:
  static int ReadFile(std::string path, std::string *result);
  static int LoadSpirv(std::string path, std::vector<unsigned char> *res);
};
