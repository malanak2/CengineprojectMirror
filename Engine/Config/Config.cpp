//
// Created by malan on 21.04.2026.
//

#include "Config.hpp"

#include <inicpp.h>
#include <memory>
#include <spdlog/spdlog.h>
namespace ini {
/** Conversion functor to parse std::vectors from an ini field-
 * The generic template can be passed down to the vector. */
template <typename T> struct Convert<std::vector<T>> {
  /** Decodes a std::vector from a string. */
  void decode(const std::string &value, std::vector<T> &result) {
    result.clear();

    // variable to store the decoded value of each element
    T decoded;
    // maintain a start and end pos within the string
    size_t startPos = 0;
    size_t endPos = 0;
    size_t cnt;

    while (endPos != std::string::npos) {
      if (endPos != 0)
        startPos = endPos + 1;
      // search for the next comma as separator
      endPos = value.find(',', startPos);

      // if no comma was found use the rest of the string
      // as input
      if (endPos == std::string::npos)
        cnt = value.size() - startPos;
      else
        cnt = endPos - startPos;

      std::string tmp = value.substr(startPos, cnt);
      // use the conversion functor for the type contained in
      // the vector, so the vector can use any type that
      // is compatible with inifile-cpp
      Convert<T> conv;
      conv.decode(tmp, decoded);
      result.push_back(decoded);
    }
  }

  /** Encodes a std::vector to a string. */
  void encode(const std::vector<T> &value, std::string &result) {
    // variable to store the encoded element value
    std::string encoded;
    // string stream to build the result stream
    std::stringstream ss;
    for (size_t i = 0; i < value.size(); ++i) {
      // use the conversion functor for the type contained in
      // the vector, so the vector can use any type that
      // is compatible with inifile-cp
      Convert<T> conv;
      conv.encode(value[i], encoded);
      ss << encoded;

      // if this is not the last element add a comma as separator
      if (i != value.size() - 1)
        ss << ',';
    }
    // store the created string in the result
    result = ss.str();
  }
};
} // namespace ini
Config::Config(std::string file_path) {
  ini::IniFile cfg;
  auto logger = spdlog::get("console");
  window = std::make_unique<Window>();
  graphics = std::make_unique<Graphics>();
  cfg.load(file_path);
  if (cfg.empty()) {
    SPDLOG_LOGGER_INFO(logger, "Config is empty, generating new file...");
    cfg["Window"]["Title"] = "New Project";
    cfg["Graphics"]["CameraPos"] = std::vector<float>{0.0, 0.0, 0.0};
    cfg["Graphics"]["CameraRot"] = std::vector<float>{0.0, 0.0, 0.0};
    cfg.save(file_path);
  }
  window->title = cfg["Window"]["Title"].as<std::string>();
  graphics->CameraPos = cfg["Graphics"]["CameraPos"].as<std::vector<float>>();
  graphics->CameraRot = cfg["Graphics"]["CameraRot"].as<std::vector<float>>();
  if (graphics->CameraPos.size() != 3 || graphics->CameraRot.size() != 3) {
    SPDLOG_LOGGER_ERROR(logger, "Invalid camera parameters specified");
  }
  SPDLOG_LOGGER_INFO(logger, "Loaded config!");
}
