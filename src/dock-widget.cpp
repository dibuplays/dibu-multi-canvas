#include "dock-widget.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

namespace dibu {

DockWidget::DockWidget(QWidget *parent) : QWidget(parent), settings_(store_.load())
{
  setObjectName(QStringLiteral("DibuMultiCanvasDock"));
  buildUi();
}

DockWidget::~DockWidget()
{
  shutdown();
}

void DockWidget::buildUi()
{
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(10, 10, 10, 10);

  auto *title = new QLabel(tr("DIBU MULTI-CANVAS STUDIO"));
  QFont titleFont = title->font();
  titleFont.setBold(true);
  titleFont.setPointSize(titleFont.pointSize() + 2);
  title->setFont(titleFont);
  root->addWidget(title);

  statusLabel_ = new QLabel;
  statusLabel_->setWordWrap(true);
  root->addWidget(statusLabel_);

  auto *canvasGroup = new QGroupBox(tr("Vertical Canvas"));
  auto *canvasForm = new QFormLayout(canvasGroup);
  enabledCheck_ = new QCheckBox(tr("Enabled"));
  enabledCheck_->setChecked(settings_.enabled);
  widthSpin_ = new QSpinBox;
  widthSpin_->setRange(320, 7680);
  widthSpin_->setSingleStep(2);
  widthSpin_->setValue(static_cast<int>(settings_.width));
  heightSpin_ = new QSpinBox;
  heightSpin_->setRange(320, 7680);
  heightSpin_->setSingleStep(2);
  heightSpin_->setValue(static_cast<int>(settings_.height));
  applyButton_ = new QPushButton(tr("Apply Canvas Settings"));
  canvasForm->addRow(enabledCheck_);
  canvasForm->addRow(tr("Width"), widthSpin_);
  canvasForm->addRow(tr("Height"), heightSpin_);
  canvasForm->addRow(applyButton_);
  root->addWidget(canvasGroup);

  auto *linkGroup = new QGroupBox(tr("Linked Scenes"));
  auto *linkLayout = new QVBoxLayout(linkGroup);
  auto *linkForm = new QFormLayout;
  masterSceneCombo_ = new QComboBox;
  canvasSceneCombo_ = new QComboBox;
  linkForm->addRow(tr("Main OBS scene"), masterSceneCombo_);
  linkForm->addRow(tr("Canvas scene"), canvasSceneCombo_);
  linkLayout->addLayout(linkForm);

  auto *buttons = new QHBoxLayout;
  auto *createButton = new QPushButton(tr("New Canvas Scene"));
  auto *linkButton = new QPushButton(tr("Link"));
  auto *unlinkButton = new QPushButton(tr("Unlink"));
  buttons->addWidget(createButton);
  buttons->addWidget(linkButton);
  buttons->addWidget(unlinkButton);
  linkLayout->addLayout(buttons);

  linkSummary_ = new QLabel;
  linkSummary_->setWordWrap(true);
  linkLayout->addWidget(linkSummary_);
  root->addWidget(linkGroup);
  root->addStretch(1);

  connect(applyButton_, &QPushButton::clicked, this, [this] { applyCanvasSettings(); });
  connect(createButton, &QPushButton::clicked, this, [this] { createCanvasScene(); });
  connect(linkButton, &QPushButton::clicked, this, [this] { linkSelectedScenes(); });
  connect(unlinkButton, &QPushButton::clicked, this, [this] { unlinkSelectedMaster(); });
}

void DockWidget::initialize()
{
  if (initialized_)
    return;
  initialized_ = true;
  refreshMasterScenes();
  if (settings_.enabled)
    canvas_.start(settings_.width, settings_.height);
  refreshCanvasScenes();
  handleMainSceneChanged();
  updateStatus();
}

void DockWidget::shutdown()
{
  if (!initialized_)
    return;
  persist();
  canvas_.stop();
  initialized_ = false;
}

void DockWidget::refreshMasterScenes()
{
  const QString selected = masterSceneCombo_->currentText();
  QSignalBlocker blocker(masterSceneCombo_);
  masterSceneCombo_->clear();

  obs_frontend_source_list scenes{};
  obs_frontend_get_scenes(&scenes);
  for (size_t i = 0; i < scenes.sources.num; ++i)
    masterSceneCombo_->addItem(QString::fromUtf8(obs_source_get_name(scenes.sources.array[i])));
  obs_frontend_source_list_free(&scenes);

  const int oldIndex = masterSceneCombo_->findText(selected);
  if (oldIndex >= 0)
    masterSceneCombo_->setCurrentIndex(oldIndex);
}

void DockWidget::refreshCanvasScenes()
{
  const QString selected = canvasSceneCombo_->currentText();
  QSignalBlocker blocker(canvasSceneCombo_);
  canvasSceneCombo_->clear();
  for (const auto &name : canvas_.scenes())
    canvasSceneCombo_->addItem(QString::fromStdString(name));
  const int oldIndex = canvasSceneCombo_->findText(selected);
  if (oldIndex >= 0)
    canvasSceneCombo_->setCurrentIndex(oldIndex);
}

std::string DockWidget::currentMainSceneName() const
{
  obs_source_t *scene = obs_frontend_get_current_scene();
  if (!scene)
    return {};
  const char *name = obs_source_get_name(scene);
  std::string result = name ? name : "";
  obs_source_release(scene);
  return result;
}

void DockWidget::handleMainSceneChanged()
{
  if (!initialized_)
    return;

  refreshMasterScenes();
  const auto mainName = currentMainSceneName();
  const int index = masterSceneCombo_->findText(QString::fromStdString(mainName));
  if (index >= 0)
    masterSceneCombo_->setCurrentIndex(index);

  if (settings_.enabled) {
    const auto linked = settings_.links.resolve(mainName);
    if (linked)
      canvas_.activateScene(*linked);
  }
  updateStatus();
}

void DockWidget::createCanvasScene()
{
  if (!canvas_.running()) {
    QMessageBox::information(this, tr("Dibu Multi-Canvas"), tr("Enable the canvas first."));
    return;
  }

  bool accepted = false;
  const QString name = QInputDialog::getText(this, tr("New Canvas Scene"), tr("Scene name"),
                                             QLineEdit::Normal, tr("Vertical - Gameplay"), &accepted).trimmed();
  if (!accepted || name.isEmpty())
    return;
  if (!canvas_.ensureScene(name.toStdString())) {
    QMessageBox::warning(this, tr("Dibu Multi-Canvas"), tr("The canvas scene could not be created."));
    return;
  }
  refreshCanvasScenes();
  canvasSceneCombo_->setCurrentText(name);
  updateStatus();
}

void DockWidget::linkSelectedScenes()
{
  const std::string master = masterSceneCombo_->currentText().toStdString();
  const std::string target = canvasSceneCombo_->currentText().toStdString();
  if (!settings_.links.link(master, target))
    return;
  persist();
  handleMainSceneChanged();
}

void DockWidget::unlinkSelectedMaster()
{
  settings_.links.unlink(masterSceneCombo_->currentText().toStdString());
  persist();
  updateStatus();
}

void DockWidget::applyCanvasSettings()
{
  settings_.enabled = enabledCheck_->isChecked();
  settings_.width = static_cast<uint32_t>(widthSpin_->value());
  settings_.height = static_cast<uint32_t>(heightSpin_->value());
  settings_.width += settings_.width & 1U;
  settings_.height += settings_.height & 1U;

  if (settings_.enabled) {
    if (!canvas_.running())
      canvas_.start(settings_.width, settings_.height);
    else
      canvas_.resetVideo(settings_.width, settings_.height);
  } else {
    canvas_.stop();
  }
  persist();
  refreshCanvasScenes();
  updateStatus();
}

void DockWidget::persist()
{
  if (!store_.save(settings_))
    blog(LOG_WARNING, "[Dibu Multi-Canvas] Could not save plugin settings");
}

void DockWidget::updateStatus()
{
  if (!canvas_.running()) {
    statusLabel_->setText(tr("Canvas stopped"));
  } else {
    statusLabel_->setText(tr("Canvas active: %1 × %2 | Scene: %3")
                            .arg(canvas_.width())
                            .arg(canvas_.height())
                            .arg(QString::fromStdString(canvas_.activeScene())));
  }

  const auto master = masterSceneCombo_->currentText().toStdString();
  const auto linked = settings_.links.resolve(master);
  linkSummary_->setText(linked ? tr("Current link: %1 → %2")
                                    .arg(QString::fromStdString(master), QString::fromStdString(*linked))
                              : tr("Current scene is not linked. %1 link(s) saved.").arg(settings_.links.size()));
}

} // namespace dibu
