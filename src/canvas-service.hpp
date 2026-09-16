#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct obs_canvas;
typedef struct obs_canvas obs_canvas_t;
struct obs_source;
typedef struct obs_source obs_source_t;

namespace dibu {

class CanvasService {
public:
  CanvasService() = default;
  ~CanvasService();

  CanvasService(const CanvasService &) = delete;
  CanvasService &operator=(const CanvasService &) = delete;

  bool start(uint32_t width, uint32_t height);
  void stop();
  bool resetVideo(uint32_t width, uint32_t height);

  bool ensureScene(const std::string &name);
  bool activateScene(const std::string &name);
  [[nodiscard]] std::vector<std::string> scenes() const;
  [[nodiscard]] std::string activeScene() const;
  [[nodiscard]] bool running() const noexcept { return canvas_ != nullptr; }
  [[nodiscard]] uint32_t width() const noexcept { return width_; }
  [[nodiscard]] uint32_t height() const noexcept { return height_; }

private:
  static bool collectScene(void *context, obs_source_t *source);
  obs_canvas_t *findExistingCanvas() const;

  obs_canvas_t *canvas_ = nullptr;
  uint32_t width_ = 1080;
  uint32_t height_ = 1920;
  std::string activeScene_;
};

} // namespace dibu
