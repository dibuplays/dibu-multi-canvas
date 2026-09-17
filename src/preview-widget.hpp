#pragma once

#include <QWidget>

#include <cstdint>

struct obs_canvas;
typedef struct obs_canvas obs_canvas_t;
struct obs_display;
typedef struct obs_display obs_display_t;

namespace dibu {

class PreviewWidget final : public QWidget {
public:
  explicit PreviewWidget(QWidget *parent = nullptr);
  ~PreviewWidget() override;

  void setCanvas(obs_canvas_t *canvas);

protected:
  void showEvent(QShowEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  QPaintEngine *paintEngine() const override;

private:
  static void draw(void *context, uint32_t width, uint32_t height);
  void createDisplay();
  void destroyDisplay();

  obs_canvas_t *canvas_ = nullptr;
  obs_display_t *display_ = nullptr;
};

} // namespace dibu
