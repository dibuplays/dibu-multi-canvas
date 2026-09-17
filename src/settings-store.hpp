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
  bool actionsEnabled = false;
  std::string microphoneSource;
  std::string webcamSource;
  std::string chatSource;
  std::string alertSource;
  double talkingThresholdDb = -35.0;
  int talkingHoldMs = 900;
  int talkingScalePercent = 135;
  int chatHoldMs = 6000;
  int alertHoldMs = 5000;
  LinkModel links;
};

class SettingsStore {
public:
  [[nodiscard]] PluginSettings load() const;
  bool save(const PluginSettings &settings) const;
};

} // namespace dibu
