#include "dedicated-preview-dock.hpp"
#include "preview-widget.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace dibu {

DedicatedPreviewDock::DedicatedPreviewDock(QWidget *parent) : QWidget(parent)
{
  setObjectName(QStringLiteral("DibuVerticalPreviewDock"));
  setMinimumSize(260, 460);
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(6, 6, 6, 6);
  preview_ = new PreviewWidget;
  preview_->setMinimumSize(240, 426);
  preview_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  status_ = new QLabel(tr("Vertical canvas stopped"));
  status_->setAlignment(Qt::AlignCenter);
  layout->addWidget(preview_, 1);
  layout->addWidget(status_);
}

void DedicatedPreviewDock::setCanvas(obs_canvas_t *canvas)
{
  preview_->setCanvas(canvas);
}

void DedicatedPreviewDock::setStatus(const QString &status)
{
  status_->setText(status);
}

} // namespace dibu
