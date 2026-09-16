#include "dock-widget.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QWidget>

#include <memory>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("dibu-multi-canvas", "en-US")

namespace {
dibu::DockWidget *g_dock = nullptr;

void frontendEvent(enum obs_frontend_event event, void *)
{
  if (!g_dock)
    return;

  switch (event) {
  case OBS_FRONTEND_EVENT_FINISHED_LOADING:
    g_dock->initialize();
    break;
  case OBS_FRONTEND_EVENT_SCENE_CHANGED:
  case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
    g_dock->handleMainSceneChanged();
    break;
  case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
    g_dock->initialize();
    break;
  case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING:
  case OBS_FRONTEND_EVENT_SCRIPTING_SHUTDOWN:
  case OBS_FRONTEND_EVENT_EXIT:
    g_dock->shutdown();
    break;
  default:
    break;
  }
}
} // namespace

const char *obs_module_name()
{
  return "Dibu Multi-Canvas Studio";
}

const char *obs_module_description()
{
  return "Independent OBS canvases with linked scene switching and future smart layout conversion.";
}

bool obs_module_load()
{
  QWidget *mainWindow = static_cast<QWidget *>(obs_frontend_get_main_window());
  auto dock = std::make_unique<dibu::DockWidget>(mainWindow);
  if (!obs_frontend_add_dock_by_id("dibu-multi-canvas", "Dibu Multi-Canvas Studio", dock.get())) {
    blog(LOG_ERROR, "[Dibu Multi-Canvas] Could not register the dock");
    return false;
  }
  g_dock = dock.release(); // The OBS dock owns the widget after successful registration.

  obs_frontend_add_event_callback(frontendEvent, nullptr);
  blog(LOG_INFO, "[Dibu Multi-Canvas] Loaded version %s", DIBU_PLUGIN_VERSION);
  return true;
}

void obs_module_unload()
{
  obs_frontend_remove_event_callback(frontendEvent, nullptr);
  if (g_dock)
    g_dock->shutdown();
  obs_frontend_remove_dock("dibu-multi-canvas");
  g_dock = nullptr;
  blog(LOG_INFO, "[Dibu Multi-Canvas] Unloaded");
}
