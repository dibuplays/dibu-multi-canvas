#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <obs.h>

struct obs_canvas;
typedef struct obs_canvas obs_canvas_t;
struct obs_source;
typedef struct obs_source obs_source_t;
struct obs_scene;
typedef struct obs_scene obs_scene_t;
struct obs_scene_item;
typedef struct obs_scene_item obs_sceneitem_t;

namespace dibu {

enum class ActionLayoutState;

class CanvasService {
public:
  struct SceneItem {
    std::string name;
    bool visible = true;
  };

  struct SourceTransform {
    double x = 0.0;
    double y = 0.0;
    double width = 1.0;
    double height = 1.0;
    double rotation = 0.0;
    int cropLeft = 0;
    int cropRight = 0;
    int cropTop = 0;
    int cropBottom = 0;
  };

  enum class SourceLayout { Fit, Fill, Center, Reset };

  CanvasService() = default;
  ~CanvasService();

  CanvasService(const CanvasService &) = delete;
  CanvasService &operator=(const CanvasService &) = delete;

  bool start(uint32_t width, uint32_t height);
  void stop();
  bool resetVideo(uint32_t width, uint32_t height);

  bool ensureScene(const std::string &name);
  bool activateScene(const std::string &name);
  bool addExistingSource(const std::string &sourceName);
  bool removeSource(const std::string &sourceName);
  bool setSourceVisible(const std::string &sourceName, bool visible);
  bool moveSource(const std::string &sourceName, bool up);
  [[nodiscard]] std::optional<SourceTransform> sourceTransform(const std::string &sourceName) const;
  bool setSourceTransform(const std::string &sourceName, const SourceTransform &transform);
  bool layoutSource(const std::string &sourceName, SourceLayout layout);
  void captureActionBaseline(const std::string &webcamSource, const std::string &chatSource,
                             const std::string &alertSource);
  void applyActionLayout(ActionLayoutState state, const std::string &webcamSource,
                         const std::string &chatSource, const std::string &alertSource,
                         float webcamScaleMultiplier);
  void clearActionLayout();
  [[nodiscard]] std::vector<std::string> scenes() const;
  [[nodiscard]] std::vector<std::string> availableSources() const;
  [[nodiscard]] std::vector<SceneItem> activeSceneItems() const;
  [[nodiscard]] std::string activeScene() const;
  [[nodiscard]] obs_canvas_t *canvas() const noexcept { return canvas_; }
  [[nodiscard]] bool running() const noexcept { return canvas_ != nullptr; }
  [[nodiscard]] uint32_t width() const noexcept { return width_; }
  [[nodiscard]] uint32_t height() const noexcept { return height_; }

private:
  static bool collectScene(void *context, obs_source_t *source);
  static bool collectAvailableSource(void *context, obs_source_t *source);
  static bool collectSceneItem(obs_scene_t *scene, obs_sceneitem_t *item, void *context);
  obs_scene_t *activeSceneRef() const;
  obs_canvas_t *findExistingCanvas() const;

  struct ActionBaseline {
    obs_transform_info transform{};
    bool visible = true;
  };
  void captureSourceBaseline(obs_scene_t *scene, const std::string &sourceName);
  void restoreSourceBaseline(obs_scene_t *scene, const std::string &sourceName);
  void setActionVisibility(obs_scene_t *scene, const std::string &sourceName, bool visible);

  obs_canvas_t *canvas_ = nullptr;
  uint32_t width_ = 1080;
  uint32_t height_ = 1920;
  std::string activeScene_;
  std::map<std::string, ActionBaseline> actionBaselines_;
};

} // namespace dibu
