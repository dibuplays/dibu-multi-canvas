#include "preview-widget.hpp"

#include <obs-module.h>
#include <graphics/graphics.h>

#include <QResizeEvent>
#include <QShowEvent>

#include <algorithm>

namespace dibu {

PreviewWidget::PreviewWidget(QWidget *parent) : QWidget(parent)
{
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_PaintOnScreen);
  setAttribute(Qt::WA_NoSystemBackground);
  setAutoFillBackground(false);
  setMinimumSize(180, 320);
}

PreviewWidget::~PreviewWidget()
{
  destroyDisplay();
}

void PreviewWidget::setCanvas(obs_canvas_t *canvas)
{
  canvas_ = canvas;
  if (canvas_ && isVisible())
    createDisplay();
}

QPaintEngine *PreviewWidget::paintEngine() const
{
  return nullptr;
}

void PreviewWidget::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  createDisplay();
}

void PreviewWidget::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  if (display_)
    obs_display_resize(display_, static_cast<uint32_t>(event->size().width()),
                       static_cast<uint32_t>(event->size().height()));
}

void PreviewWidget::createDisplay()
{
#ifdef _WIN32
  if (display_ || !canvas_ || width() <= 0 || height() <= 0)
    return;

  gs_init_data info{};
  info.cx = static_cast<uint32_t>(width());
  info.cy = static_cast<uint32_t>(height());
  info.format = GS_BGRA;
  info.zsformat = GS_ZS_NONE;
  info.window.hwnd = reinterpret_cast<void *>(winId());

  display_ = obs_display_create(&info, 0x17191f);
  if (display_)
    obs_display_add_draw_callback(display_, draw, this);
  else
    blog(LOG_ERROR, "[Dibu Multi-Canvas] Could not create the vertical preview display");
#endif
}

void PreviewWidget::destroyDisplay()
{
  if (!display_)
    return;
  obs_display_remove_draw_callback(display_, draw, this);
  obs_display_destroy(display_);
  display_ = nullptr;
}

void PreviewWidget::draw(void *context, uint32_t width, uint32_t height)
{
  auto *preview = static_cast<PreviewWidget *>(context);
  if (!preview || !preview->canvas_)
    return;

  obs_video_info info{};
  if (!obs_canvas_get_video_info(preview->canvas_, &info) || !info.base_width || !info.base_height)
    return;

  const float scale = std::min(static_cast<float>(width) / info.base_width,
                               static_cast<float>(height) / info.base_height);
  const uint32_t drawWidth = static_cast<uint32_t>(info.base_width * scale);
  const uint32_t drawHeight = static_cast<uint32_t>(info.base_height * scale);
  const int x = static_cast<int>((width - drawWidth) / 2U);
  const int y = static_cast<int>((height - drawHeight) / 2U);

  gs_viewport_push();
  gs_projection_push();
  gs_set_viewport(x, y, static_cast<int>(drawWidth), static_cast<int>(drawHeight));
  gs_ortho(0.0f, static_cast<float>(info.base_width), 0.0f, static_cast<float>(info.base_height), -100.0f,
           100.0f);
  obs_canvas_render(preview->canvas_);
  gs_projection_pop();
  gs_viewport_pop();
}

} // namespace dibu
