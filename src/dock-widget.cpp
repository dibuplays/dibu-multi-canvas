#include "dock-widget.hpp"
#include "preview-widget.hpp"

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
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStringList>
#include <QTimer>
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
  auto *outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  auto *scroll = new QScrollArea;
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto *content = new QWidget;
  auto *root = new QVBoxLayout(content);
  root->setContentsMargins(10, 10, 10, 10);
  scroll->setWidget(content);
  outer->addWidget(scroll);

  auto *title = new QLabel(tr("DIBU MULTI-CANVAS STUDIO"));
  QFont titleFont = title->font();
  titleFont.setBold(true);
  titleFont.setPointSize(titleFont.pointSize() + 2);
  title->setFont(titleFont);
  root->addWidget(title);

  statusLabel_ = new QLabel;
  statusLabel_->setWordWrap(true);
  root->addWidget(statusLabel_);

  auto *previewGroup = new QGroupBox(tr("Vertical Preview"));
  auto *previewLayout = new QVBoxLayout(previewGroup);
  preview_ = new PreviewWidget;
  previewLayout->addWidget(preview_, 1);
  root->addWidget(previewGroup, 1);

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

  auto *sourceGroup = new QGroupBox(tr("Vertical Sources"));
  auto *sourceLayout = new QVBoxLayout(sourceGroup);
  sourceList_ = new QListWidget;
  sourceList_->setMinimumHeight(110);
  sourceLayout->addWidget(sourceList_);
  auto *sourceButtons = new QHBoxLayout;
  auto *addSourceButton = new QPushButton(tr("Add Existing"));
  auto *removeSourceButton = new QPushButton(tr("Remove"));
  auto *visibilityButton = new QPushButton(tr("Show/Hide"));
  auto *moveUpButton = new QPushButton(tr("↑"));
  auto *moveDownButton = new QPushButton(tr("↓"));
  sourceButtons->addWidget(addSourceButton);
  sourceButtons->addWidget(removeSourceButton);
  sourceButtons->addWidget(visibilityButton);
  sourceButtons->addWidget(moveUpButton);
  sourceButtons->addWidget(moveDownButton);
  sourceLayout->addLayout(sourceButtons);
  root->addWidget(sourceGroup);

  auto *outputGroup = new QGroupBox(tr("Vertical Outputs"));
  auto *outputLayout = new QVBoxLayout(outputGroup);
  recordButton_ = new QPushButton(tr("Start Vertical Recording"));
  outputLayout->addWidget(recordButton_);
  auto *streamForm = new QFormLayout;
  serverEdit_ = new QLineEdit(QString::fromStdString(settings_.streamServer));
  serverEdit_->setPlaceholderText(tr("rtmp://server/app"));
  keyEdit_ = new QLineEdit;
  keyEdit_->setEchoMode(QLineEdit::Password);
  keyEdit_->setPlaceholderText(tr("Stream key (not saved)"));
  streamForm->addRow(tr("RTMP server"), serverEdit_);
  streamForm->addRow(tr("Stream key"), keyEdit_);
  outputLayout->addLayout(streamForm);
  streamButton_ = new QPushButton(tr("Start Vertical Stream"));
  outputLayout->addWidget(streamButton_);
  outputStatus_ = new QLabel(tr("Outputs stopped"));
  outputStatus_->setWordWrap(true);
  outputLayout->addWidget(outputStatus_);
  root->addWidget(outputGroup);

  connect(applyButton_, &QPushButton::clicked, this, [this] { applyCanvasSettings(); });
  connect(createButton, &QPushButton::clicked, this, [this] { createCanvasScene(); });
  connect(linkButton, &QPushButton::clicked, this, [this] { linkSelectedScenes(); });
  connect(unlinkButton, &QPushButton::clicked, this, [this] { unlinkSelectedMaster(); });
  connect(canvasSceneCombo_, &QComboBox::currentTextChanged, this,
          [this] { activateSelectedCanvasScene(); });
  connect(addSourceButton, &QPushButton::clicked, this, [this] { addExistingSource(); });
  connect(removeSourceButton, &QPushButton::clicked, this, [this] { removeSelectedSource(); });
  connect(visibilityButton, &QPushButton::clicked, this, [this] { toggleSelectedSourceVisibility(); });
  connect(moveUpButton, &QPushButton::clicked, this, [this] { moveSelectedSource(true); });
  connect(moveDownButton, &QPushButton::clicked, this, [this] { moveSelectedSource(false); });
  connect(sourceList_, &QListWidget::itemChanged, this, [this](QListWidgetItem *item) {
    if (!item)
      return;
    canvas_.setSourceVisible(item->data(Qt::UserRole).toString().toStdString(),
                             item->checkState() == Qt::Checked);
  });
  connect(recordButton_, &QPushButton::clicked, this, [this] { toggleRecording(); });
  connect(streamButton_, &QPushButton::clicked, this, [this] { toggleStreaming(); });

  statusTimer_ = new QTimer(this);
  statusTimer_->setInterval(500);
  connect(statusTimer_, &QTimer::timeout, this, [this] { updateStatus(); });
}

void DockWidget::initialize()
{
  if (initialized_)
    return;
  initialized_ = true;
  refreshMasterScenes();
  if (settings_.enabled)
    canvas_.start(settings_.width, settings_.height);
  preview_->setCanvas(canvas_.canvas());
  refreshCanvasScenes();
  refreshCanvasSources();
  handleMainSceneChanged();
  statusTimer_->start();
  updateStatus();
}

void DockWidget::shutdown()
{
  if (!initialized_)
    return;
  statusTimer_->stop();
  persist();
  outputs_.shutdown();
  preview_->setCanvas(nullptr);
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

void DockWidget::refreshCanvasSources()
{
  if (!sourceList_)
    return;
  const QString selected = sourceList_->currentItem() ? sourceList_->currentItem()->data(Qt::UserRole).toString()
                                                      : QString{};
  QSignalBlocker blocker(sourceList_);
  sourceList_->clear();
  for (const auto &entry : canvas_.activeSceneItems()) {
    auto *item = new QListWidgetItem(QString::fromStdString(entry.name), sourceList_);
    item->setData(Qt::UserRole, QString::fromStdString(entry.name));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(entry.visible ? Qt::Checked : Qt::Unchecked);
    if (item->data(Qt::UserRole).toString() == selected)
      sourceList_->setCurrentItem(item);
  }
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
    if (linked) {
      canvas_.activateScene(*linked);
      QSignalBlocker blocker(canvasSceneCombo_);
      canvasSceneCombo_->setCurrentText(QString::fromStdString(*linked));
      refreshCanvasSources();
    }
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
  canvas_.activateScene(name.toStdString());
  refreshCanvasSources();
  updateStatus();
}

void DockWidget::activateSelectedCanvasScene()
{
  const auto name = canvasSceneCombo_->currentText().toStdString();
  if (!name.empty())
    canvas_.activateScene(name);
  refreshCanvasSources();
  updateStatus();
}

void DockWidget::addExistingSource()
{
  const auto sourceNames = canvas_.availableSources();
  if (sourceNames.empty()) {
    QMessageBox::information(this, tr("Dibu Multi-Canvas"), tr("No reusable OBS input sources were found."));
    return;
  }

  QStringList choices;
  for (const auto &name : sourceNames)
    choices.push_back(QString::fromStdString(name));
  bool accepted = false;
  const QString selected = QInputDialog::getItem(this, tr("Add Existing Source"), tr("OBS source"), choices, 0,
                                                  false, &accepted);
  if (!accepted || selected.isEmpty())
    return;
  if (!canvas_.addExistingSource(selected.toStdString())) {
    QMessageBox::warning(this, tr("Dibu Multi-Canvas"),
                         tr("The source could not be added to the active vertical scene."));
    return;
  }
  refreshCanvasSources();
}

void DockWidget::removeSelectedSource()
{
  auto *item = sourceList_->currentItem();
  if (!item)
    return;
  canvas_.removeSource(item->data(Qt::UserRole).toString().toStdString());
  refreshCanvasSources();
}

void DockWidget::moveSelectedSource(bool up)
{
  auto *item = sourceList_->currentItem();
  if (!item)
    return;
  const QString name = item->data(Qt::UserRole).toString();
  canvas_.moveSource(name.toStdString(), up);
  refreshCanvasSources();
  for (int i = 0; i < sourceList_->count(); ++i) {
    if (sourceList_->item(i)->data(Qt::UserRole).toString() == name) {
      sourceList_->setCurrentRow(i);
      break;
    }
  }
}

void DockWidget::toggleSelectedSourceVisibility()
{
  auto *item = sourceList_->currentItem();
  if (!item)
    return;
  item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
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
    outputs_.shutdown();
    canvas_.stop();
  }
  preview_->setCanvas(canvas_.canvas());
  persist();
  refreshCanvasScenes();
  if (canvasSceneCombo_->count() > 0 && canvas_.activeScene().empty())
    canvas_.activateScene(canvasSceneCombo_->currentText().toStdString());
  refreshCanvasSources();
  updateStatus();
}

void DockWidget::toggleRecording()
{
  if (outputs_.recording()) {
    outputs_.stopRecording();
  } else if (!outputs_.startRecording(canvas_.canvas())) {
    QMessageBox::warning(this, tr("Vertical Recording"), QString::fromStdString(outputs_.lastError()));
  }
  updateStatus();
}

void DockWidget::toggleStreaming()
{
  if (outputs_.streaming()) {
    outputs_.stopStreaming();
  } else {
    settings_.streamServer = serverEdit_->text().trimmed().toStdString();
    persist();
    if (!outputs_.startStreaming(canvas_.canvas(), settings_.streamServer, keyEdit_->text().toStdString()))
      QMessageBox::warning(this, tr("Vertical Streaming"), QString::fromStdString(outputs_.lastError()));
  }
  updateStatus();
}

void DockWidget::persist()
{
  if (serverEdit_)
    settings_.streamServer = serverEdit_->text().trimmed().toStdString();
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

  if (recordButton_)
    recordButton_->setText(outputs_.recording() ? tr("Stop Vertical Recording") : tr("Start Vertical Recording"));
  if (streamButton_)
    streamButton_->setText(outputs_.streaming() ? tr("Stop Vertical Stream") : tr("Start Vertical Stream"));
  if (outputStatus_) {
    QStringList states;
    if (outputs_.recording())
      states << tr("Recording: %1").arg(QString::fromStdString(outputs_.recordingPath()));
    if (outputs_.streaming())
      states << tr("Vertical stream live");
    outputStatus_->setText(states.isEmpty() ? tr("Outputs stopped") : states.join(QStringLiteral("\n")));
  }
}

} // namespace dibu
