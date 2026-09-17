#include "settings-store.hpp"

#include <obs-module.h>
#include <util/bmem.h>
#include <util/platform.h>

#include <string>

namespace dibu {
namespace {
constexpr const char *kSettingsFile = "settings.json";

std::string configPath()
{
  char *directory = obs_module_config_path("");
  if (directory && *directory)
    os_mkdirs(directory);
  bfree(directory);

  char *raw = obs_module_config_path(kSettingsFile);
  std::string path = raw ? raw : "";
  bfree(raw);
  return path;
}
} // namespace

PluginSettings SettingsStore::load() const
{
  PluginSettings result;
  const auto path = configPath();
  if (path.empty())
    return result;

  obs_data_t *root = obs_data_create_from_json_file_safe(path.c_str(), "bak");
  if (!root)
    return result;

  const auto width = obs_data_get_int(root, "width");
  const auto height = obs_data_get_int(root, "height");
  if (width >= 320 && width <= 7680)
    result.width = static_cast<uint32_t>(width);
  if (height >= 320 && height <= 7680)
    result.height = static_cast<uint32_t>(height);
  if (obs_data_has_user_value(root, "enabled"))
    result.enabled = obs_data_get_bool(root, "enabled");
  result.streamServer = obs_data_get_string(root, "stream_server");
  result.actionsEnabled = obs_data_get_bool(root, "actions_enabled");
  result.microphoneSource = obs_data_get_string(root, "microphone_source");
  result.webcamSource = obs_data_get_string(root, "webcam_source");
  result.chatSource = obs_data_get_string(root, "chat_source");
  result.alertSource = obs_data_get_string(root, "alert_source");
  if (obs_data_has_user_value(root, "talking_threshold_db"))
    result.talkingThresholdDb = obs_data_get_double(root, "talking_threshold_db");
  if (obs_data_has_user_value(root, "talking_hold_ms"))
    result.talkingHoldMs = static_cast<int>(obs_data_get_int(root, "talking_hold_ms"));
  if (obs_data_has_user_value(root, "talking_scale_percent"))
    result.talkingScalePercent = static_cast<int>(obs_data_get_int(root, "talking_scale_percent"));
  if (obs_data_has_user_value(root, "chat_hold_ms"))
    result.chatHoldMs = static_cast<int>(obs_data_get_int(root, "chat_hold_ms"));
  if (obs_data_has_user_value(root, "alert_hold_ms"))
    result.alertHoldMs = static_cast<int>(obs_data_get_int(root, "alert_hold_ms"));

  if (obs_data_array_t *links = obs_data_get_array(root, "links")) {
    const auto count = obs_data_array_count(links);
    for (size_t i = 0; i < count; ++i) {
      obs_data_t *item = obs_data_array_item(links, i);
      result.links.link(obs_data_get_string(item, "master"), obs_data_get_string(item, "canvas"));
      obs_data_release(item);
    }
    obs_data_array_release(links);
  }
  obs_data_release(root);
  return result;
}

bool SettingsStore::save(const PluginSettings &settings) const
{
  const auto path = configPath();
  if (path.empty())
    return false;

  obs_data_t *root = obs_data_create();
  obs_data_set_int(root, "width", settings.width);
  obs_data_set_int(root, "height", settings.height);
  obs_data_set_bool(root, "enabled", settings.enabled);
  obs_data_set_string(root, "stream_server", settings.streamServer.c_str());
  obs_data_set_bool(root, "actions_enabled", settings.actionsEnabled);
  obs_data_set_string(root, "microphone_source", settings.microphoneSource.c_str());
  obs_data_set_string(root, "webcam_source", settings.webcamSource.c_str());
  obs_data_set_string(root, "chat_source", settings.chatSource.c_str());
  obs_data_set_string(root, "alert_source", settings.alertSource.c_str());
  obs_data_set_double(root, "talking_threshold_db", settings.talkingThresholdDb);
  obs_data_set_int(root, "talking_hold_ms", settings.talkingHoldMs);
  obs_data_set_int(root, "talking_scale_percent", settings.talkingScalePercent);
  obs_data_set_int(root, "chat_hold_ms", settings.chatHoldMs);
  obs_data_set_int(root, "alert_hold_ms", settings.alertHoldMs);

  obs_data_array_t *links = obs_data_array_create();
  for (const auto &[master, canvas] : settings.links.links()) {
    obs_data_t *item = obs_data_create();
    obs_data_set_string(item, "master", master.c_str());
    obs_data_set_string(item, "canvas", canvas.c_str());
    obs_data_array_push_back(links, item);
    obs_data_release(item);
  }
  obs_data_set_array(root, "links", links);
  obs_data_array_release(links);

  const bool saved = obs_data_save_json_safe(root, path.c_str(), "tmp", "bak");
  obs_data_release(root);
  return saved;
}

} // namespace dibu
