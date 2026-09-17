#include "canvas-service.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <algorithm>
#include <cstring>

namespace dibu {
namespace {
constexpr const char *kCanvasName = "Dibu Multi-Canvas - Vertical";
constexpr const char *kDefaultScene = "Vertical - Default";

uint32_t even(uint32_t value)
{
  return value + (value & 1U);
}
} // namespace

CanvasService::~CanvasService()
{
  stop();
}

obs_canvas_t *CanvasService::findExistingCanvas() const
{
  obs_frontend_canvas_list list{};
  obs_frontend_get_canvases(&list);
  obs_canvas_t *found = nullptr;
  for (size_t i = 0; i < list.canvases.num; ++i) {
    obs_canvas_t *candidate = list.canvases.array[i];
    if (!obs_canvas_removed(candidate) && std::strcmp(obs_canvas_get_name(candidate), kCanvasName) == 0) {
      found = obs_canvas_get_ref(candidate);
      break;
    }
  }
  obs_frontend_canvas_list_free(&list);
  return found;
}

bool CanvasService::start(uint32_t width, uint32_t height)
{
  width = even(std::clamp(width, 320U, 7680U));
  height = even(std::clamp(height, 320U, 7680U));

  if (!canvas_)
    canvas_ = findExistingCanvas();

  if (!canvas_) {
    obs_video_info video{};
    if (!obs_get_video_info(&video)) {
      blog(LOG_ERROR, "[Dibu Multi-Canvas] Could not read the main OBS video configuration");
      return false;
    }
    video.base_width = width;
    video.base_height = height;
    video.output_width = width;
    video.output_height = height;
    canvas_ = obs_frontend_add_canvas(kCanvasName, &video, PROGRAM);
  }

  if (!canvas_) {
    blog(LOG_ERROR, "[Dibu Multi-Canvas] Could not create the vertical canvas");
    return false;
  }

  if (!resetVideo(width, height)) {
    blog(LOG_ERROR, "[Dibu Multi-Canvas] Could not start the vertical video mix");
    stop();
    return false;
  }

  ensureScene(kDefaultScene);
  if (activeScene_.empty())
    activateScene(kDefaultScene);

  blog(LOG_INFO, "[Dibu Multi-Canvas] Canvas ready at %ux%u", width_, height_);
  return true;
}

void CanvasService::stop()
{
  if (!canvas_)
    return;

  obs_canvas_set_channel(canvas_, 0, nullptr);
  if (!obs_frontend_remove_canvas(canvas_))
    obs_canvas_remove(canvas_);
  obs_canvas_release(canvas_);
  canvas_ = nullptr;
  activeScene_.clear();
}

bool CanvasService::resetVideo(uint32_t width, uint32_t height)
{
  if (!canvas_)
    return false;

  width = even(std::clamp(width, 320U, 7680U));
  height = even(std::clamp(height, 320U, 7680U));

  obs_video_info video{};
  if (!obs_get_video_info(&video))
    return false;
  video.base_width = width;
  video.base_height = height;
  video.output_width = width;
  video.output_height = height;

  if (!obs_canvas_reset_video(canvas_, &video))
    return false;

  width_ = width;
  height_ = height;
  return true;
}

bool CanvasService::ensureScene(const std::string &name)
{
  if (!canvas_ || name.empty())
    return false;

  if (obs_scene_t *existing = obs_canvas_get_scene_by_name(canvas_, name.c_str())) {
    obs_scene_release(existing);
    return true;
  }

  obs_scene_t *created = obs_canvas_scene_create(canvas_, name.c_str());
  if (!created)
    return false;
  obs_scene_release(created);
  return true;
}

bool CanvasService::activateScene(const std::string &name)
{
  if (!canvas_ || name.empty())
    return false;

  obs_scene_t *scene = obs_canvas_get_scene_by_name(canvas_, name.c_str());
  if (!scene)
    return false;

  obs_source_t *source = obs_scene_get_source(scene);
  obs_canvas_set_channel(canvas_, 0, source);
  activeScene_ = name;
  obs_scene_release(scene);
  return true;
}

obs_scene_t *CanvasService::activeSceneRef() const
{
  if (!canvas_ || activeScene_.empty())
    return nullptr;
  return obs_canvas_get_scene_by_name(canvas_, activeScene_.c_str());
}

bool CanvasService::addExistingSource(const std::string &sourceName)
{
  obs_scene_t *scene = activeSceneRef();
  if (!scene || sourceName.empty())
    return false;

  if (obs_scene_find_source(scene, sourceName.c_str())) {
    obs_scene_release(scene);
    return true;
  }

  obs_source_t *source = obs_get_source_by_name(sourceName.c_str());
  if (!source) {
    obs_scene_release(scene);
    return false;
  }

  obs_sceneitem_t *item = obs_scene_add(scene, source);
  obs_source_release(source);
  obs_scene_release(scene);
  return item != nullptr;
}

bool CanvasService::removeSource(const std::string &sourceName)
{
  obs_scene_t *scene = activeSceneRef();
  if (!scene)
    return false;
  obs_sceneitem_t *item = obs_scene_find_source(scene, sourceName.c_str());
  if (item)
    obs_sceneitem_remove(item);
  obs_scene_release(scene);
  return item != nullptr;
}

bool CanvasService::setSourceVisible(const std::string &sourceName, bool visible)
{
  obs_scene_t *scene = activeSceneRef();
  if (!scene)
    return false;
  obs_sceneitem_t *item = obs_scene_find_source(scene, sourceName.c_str());
  const bool changed = item && obs_sceneitem_set_visible(item, visible);
  obs_scene_release(scene);
  return changed;
}

bool CanvasService::moveSource(const std::string &sourceName, bool up)
{
  obs_scene_t *scene = activeSceneRef();
  if (!scene)
    return false;
  obs_sceneitem_t *item = obs_scene_find_source(scene, sourceName.c_str());
  if (item)
    obs_sceneitem_set_order(item, up ? OBS_ORDER_MOVE_UP : OBS_ORDER_MOVE_DOWN);
  obs_scene_release(scene);
  return item != nullptr;
}

bool CanvasService::collectScene(void *context, obs_source_t *source)
{
  auto *result = static_cast<std::vector<std::string> *>(context);
  const char *name = obs_source_get_name(source);
  if (name && *name)
    result->emplace_back(name);
  return true;
}

std::vector<std::string> CanvasService::scenes() const
{
  std::vector<std::string> result;
  if (canvas_)
    obs_canvas_enum_scenes(canvas_, collectScene, &result);
  std::sort(result.begin(), result.end());
  return result;
}

bool CanvasService::collectAvailableSource(void *context, obs_source_t *source)
{
  auto *result = static_cast<std::vector<std::string> *>(context);
  if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
    return true;
  const char *name = obs_source_get_name(source);
  if (name && *name)
    result->emplace_back(name);
  return true;
}

std::vector<std::string> CanvasService::availableSources() const
{
  std::vector<std::string> result;
  obs_enum_sources(collectAvailableSource, &result);
  std::sort(result.begin(), result.end());
  return result;
}

bool CanvasService::collectSceneItem(obs_scene_t *, obs_sceneitem_t *item, void *context)
{
  auto *result = static_cast<std::vector<SceneItem> *>(context);
  obs_source_t *source = obs_sceneitem_get_source(item);
  const char *name = source ? obs_source_get_name(source) : nullptr;
  if (name && *name)
    result->push_back({name, obs_sceneitem_visible(item)});
  return true;
}

std::vector<CanvasService::SceneItem> CanvasService::activeSceneItems() const
{
  std::vector<SceneItem> result;
  obs_scene_t *scene = activeSceneRef();
  if (!scene)
    return result;
  obs_scene_enum_items(scene, collectSceneItem, &result);
  obs_scene_release(scene);
  return result;
}

std::string CanvasService::activeScene() const
{
  return activeScene_;
}

} // namespace dibu
