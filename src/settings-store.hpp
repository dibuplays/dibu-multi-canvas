#pragma once

#include "link-model.hpp"

#include <cstdint>
#include <string>

namespace dibu {

struct PluginSettings {
  uint32_t width = 1080;
  uint32_t height = 1920;
  bool enabled = true;
  std::string streamServer;
  LinkModel links;
};

class SettingsStore {
public:
  [[nodiscard]] PluginSettings load() const;
  bool save(const PluginSettings &settings) const;
};

} // namespace dibu
