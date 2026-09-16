#pragma once

#include "canvas-service.hpp"
#include "settings-store.hpp"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

namespace dibu {

class DockWidget final : public QWidget {
public:
  explicit DockWidget(QWidget *parent = nullptr);
  ~DockWidget() override;

  void initialize();
  void shutdown();
  void handleMainSceneChanged();

private:
  void buildUi();
  void refreshMasterScenes();
  void refreshCanvasScenes();
  void updateStatus();
  void createCanvasScene();
  void linkSelectedScenes();
  void unlinkSelectedMaster();
  void applyCanvasSettings();
  void persist();
  [[nodiscard]] std::string currentMainSceneName() const;

  CanvasService canvas_;
  SettingsStore store_;
  PluginSettings settings_;
  bool initialized_ = false;

  QLabel *statusLabel_ = nullptr;
  QCheckBox *enabledCheck_ = nullptr;
  QSpinBox *widthSpin_ = nullptr;
  QSpinBox *heightSpin_ = nullptr;
  QComboBox *masterSceneCombo_ = nullptr;
  QComboBox *canvasSceneCombo_ = nullptr;
  QLabel *linkSummary_ = nullptr;
  QPushButton *applyButton_ = nullptr;
};

} // namespace dibu
