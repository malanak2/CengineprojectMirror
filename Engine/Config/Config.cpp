//
// Created by malan on 21.04.2026.
//

#include "Config.hpp"

#include <inicpp.h>
#include <spdlog/spdlog.h>
Config::Config(std::string file_path) {
  ini::IniFile cfg;
  auto logger = spdlog::get("console");
  cfg.load(file_path);
  if (cfg.empty()) {
    logger->info("Config is empty, generating new file...");
    cfg["Window"]["Title"] = "New Project";
    cfg.save(file_path);
  }
  Window_Title = cfg["Window"]["Title"].as<std::string>();
  logger->info("Loaded config!");
}
