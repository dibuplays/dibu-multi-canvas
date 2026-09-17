#pragma once

#include <QWidget>

struct obs_canvas;
typedef struct obs_canvas obs_canvas_t;

class QLabel;

namespace dibu {

class PreviewWidget;

class DedicatedPreviewDock final : public QWidget {
public:
  explicit DedicatedPreviewDock(QWidget *parent = nullptr);
  void setCanvas(obs_canvas_t *canvas);
  void setStatus(const QString &status);

private:
  PreviewWidget *preview_ = nullptr;
  QLabel *status_ = nullptr;
};

} // namespace dibu
