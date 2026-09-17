#pragma once

#include "canvas-service.hpp"
#include "output-service.hpp"
#include "settings-store.hpp"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTimer;

namespace dibu {
class PreviewWidget;
}

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
  void refreshCanvasSources();
  void updateStatus();
  void createCanvasScene();
  void linkSelectedScenes();
  void unlinkSelectedMaster();
  void applyCanvasSettings();
  void activateSelectedCanvasScene();
  void addExistingSource();
  void removeSelectedSource();
  void moveSelectedSource(bool up);
  void toggleSelectedSourceVisibility();
  void toggleRecording();
  void toggleStreaming();
  void persist();
  [[nodiscard]] std::string currentMainSceneName() const;

  CanvasService canvas_;
  OutputService outputs_;
  SettingsStore store_;
  PluginSettings settings_;
  bool initialized_ = false;

  QLabel *statusLabel_ = nullptr;
  QCheckBox *enabledCheck_ = nullptr;
  QSpinBox *widthSpin_ = nullptr;
  QSpinBox *heightSpin_ = nullptr;
  QComboBox *masterSceneCombo_ = nullptr;
  QComboBox *canvasSceneCombo_ = nullptr;
  PreviewWidget *preview_ = nullptr;
  QListWidget *sourceList_ = nullptr;
  QLabel *linkSummary_ = nullptr;
  QPushButton *applyButton_ = nullptr;
  QPushButton *recordButton_ = nullptr;
  QPushButton *streamButton_ = nullptr;
  QLabel *outputStatus_ = nullptr;
  QLineEdit *serverEdit_ = nullptr;
  QLineEdit *keyEdit_ = nullptr;
  QTimer *statusTimer_ = nullptr;
};

} // namespace dibu
