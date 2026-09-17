#include "action-layout-controller.hpp"

#include <obs-audio-controls.h>
#include <obs-module.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace dibu {

ActionLayoutController::~ActionLayoutController()
{
  shutdown();
}

void ActionLayoutController::configure(const ActionLayoutConfig &config)
{
  const bool microphoneChanged = config.microphoneSource != config_.microphoneSource;
  config_ = config;
  if (!config_.enabled) {
    shutdown();
    config_ = config;
    return;
  }
  if (microphoneChanged || !volmeter_) {
    detachMicrophone();
    attachMicrophone();
  }
  previousChatActive_ = sourceActive(config_.chatSource);
  previousAlertActive_ = sourceActive(config_.alertSource);
}

void ActionLayoutController::shutdown()
{
  detachMicrophone();
  microphonePeakDb_.store(-96.0f);
  talkingUntil_ = {};
  chatUntil_ = {};
  alertUntil_ = {};
  previousChatActive_ = false;
  previousAlertActive_ = false;
  cutscene_ = false;
  state_ = ActionLayoutState::Normal;
}

void ActionLayoutController::attachMicrophone()
{
  if (!config_.enabled || config_.microphoneSource.empty())
    return;
  obs_source_t *source = obs_get_source_by_name(config_.microphoneSource.c_str());
  if (!source)
    return;
  volmeter_ = obs_volmeter_create(OBS_FADER_LOG);
  if (volmeter_) {
    obs_volmeter_add_callback(volmeter_, volumeUpdated, this);
    if (!obs_volmeter_attach_source(volmeter_, source)) {
      obs_volmeter_remove_callback(volmeter_, volumeUpdated, this);
      obs_volmeter_destroy(volmeter_);
      volmeter_ = nullptr;
    }
  }
  obs_source_release(source);
}

void ActionLayoutController::detachMicrophone()
{
  if (!volmeter_)
    return;
  obs_volmeter_remove_callback(volmeter_, volumeUpdated, this);
  obs_volmeter_detach_source(volmeter_);
  obs_volmeter_destroy(volmeter_);
  volmeter_ = nullptr;
}

void ActionLayoutController::volumeUpdated(void *context, const float[], const float peak[], const float[])
{
  auto *controller = static_cast<ActionLayoutController *>(context);
  if (!controller || !peak)
    return;
  float highest = -96.0f;
  for (size_t channel = 0; channel < MAX_AUDIO_CHANNELS; ++channel) {
    if (std::isfinite(peak[channel]))
      highest = std::max(highest, peak[channel]);
  }
  controller->microphonePeakDb_.store(highest);
}

bool ActionLayoutController::sourceActive(const std::string &name) const
{
  if (name.empty())
    return false;
  obs_source_t *source = obs_get_source_by_name(name.c_str());
  if (!source)
    return false;
  const bool active = obs_source_active(source);
  obs_source_release(source);
  return active;
}

ActionLayoutState ActionLayoutController::tick()
{
  if (!config_.enabled) {
    state_ = ActionLayoutState::Normal;
    return state_;
  }

  const auto now = Clock::now();
  if (microphonePeakDb_.load() >= config_.talkingThresholdDb)
    talkingUntil_ = now + std::chrono::milliseconds(std::max(0, config_.talkingHoldMs));

  const bool chatActive = sourceActive(config_.chatSource);
  if (chatActive && !previousChatActive_)
    triggerChat();
  previousChatActive_ = chatActive;

  const bool alertActive = sourceActive(config_.alertSource);
  if (alertActive && !previousAlertActive_)
    triggerAlert();
  previousAlertActive_ = alertActive;

  if (cutscene_)
    state_ = ActionLayoutState::Cutscene;
  else if (now < alertUntil_)
    state_ = ActionLayoutState::Alert;
  else if (now < chatUntil_)
    state_ = ActionLayoutState::Chat;
  else if (now < talkingUntil_)
    state_ = ActionLayoutState::Talking;
  else
    state_ = ActionLayoutState::Normal;
  return state_;
}

void ActionLayoutController::triggerChat()
{
  chatUntil_ = Clock::now() + std::chrono::milliseconds(std::max(0, config_.chatHoldMs));
}

void ActionLayoutController::triggerAlert()
{
  alertUntil_ = Clock::now() + std::chrono::milliseconds(std::max(0, config_.alertHoldMs));
}

void ActionLayoutController::toggleCutscene()
{
  setCutscene(!cutscene_);
}

void ActionLayoutController::setCutscene(bool enabled)
{
  cutscene_ = enabled;
}

const char *ActionLayoutController::stateName(ActionLayoutState state)
{
  switch (state) {
  case ActionLayoutState::Talking:
    return "Talking";
  case ActionLayoutState::Chat:
    return "Chat";
  case ActionLayoutState::Alert:
    return "Alert";
  case ActionLayoutState::Cutscene:
    return "Cutscene";
  case ActionLayoutState::Normal:
  default:
    return "Normal";
  }
}

} // namespace dibu
