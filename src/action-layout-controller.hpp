#pragma once

#include <atomic>
#include <chrono>
#include <string>

struct obs_volmeter;
typedef struct obs_volmeter obs_volmeter_t;

namespace dibu {

enum class ActionLayoutState { Normal, Talking, Chat, Alert, Cutscene };

struct ActionLayoutConfig {
  bool enabled = false;
  std::string microphoneSource;
  std::string chatSource;
  std::string alertSource;
  float talkingThresholdDb = -35.0f;
  int talkingHoldMs = 900;
  int chatHoldMs = 6000;
  int alertHoldMs = 5000;
};

class ActionLayoutController {
public:
  ActionLayoutController() = default;
  ~ActionLayoutController();

  ActionLayoutController(const ActionLayoutController &) = delete;
  ActionLayoutController &operator=(const ActionLayoutController &) = delete;

  void configure(const ActionLayoutConfig &config);
  void shutdown();
  ActionLayoutState tick();
  void triggerChat();
  void triggerAlert();
  void toggleCutscene();
  void setCutscene(bool enabled);

  [[nodiscard]] ActionLayoutState state() const noexcept { return state_; }
  [[nodiscard]] bool cutscene() const noexcept { return cutscene_; }
  [[nodiscard]] float microphonePeakDb() const noexcept { return microphonePeakDb_.load(); }
  [[nodiscard]] static const char *stateName(ActionLayoutState state);

private:
  using Clock = std::chrono::steady_clock;
  static void volumeUpdated(void *context, const float magnitude[], const float peak[], const float inputPeak[]);
  void attachMicrophone();
  void detachMicrophone();
  bool sourceActive(const std::string &name) const;

  ActionLayoutConfig config_;
  obs_volmeter_t *volmeter_ = nullptr;
  std::atomic<float> microphonePeakDb_{-96.0f};
  Clock::time_point talkingUntil_{};
  Clock::time_point chatUntil_{};
  Clock::time_point alertUntil_{};
  bool previousChatActive_ = false;
  bool previousAlertActive_ = false;
  bool cutscene_ = false;
  ActionLayoutState state_ = ActionLayoutState::Normal;
};

} // namespace dibu
