#pragma once

#include "canvas-service.hpp"
#include "action-layout-controller.hpp"
#include "output-service.hpp"
#include "settings-store.hpp"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTimer;

namespace dibu {
class PreviewWidget;
class DedicatedPreviewDock;
}

namespace dibu {

class DockWidget final : public QWidget {
public:
  explicit DockWidget(QWidget *parent = nullptr);
  ~DockWidget() override;

  void initialize();
  void shutdown();
  void handleMainSceneChanged();
  void setDedicatedPreview(DedicatedPreviewDock *preview);

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
  void refreshActionSources();
  void applyActionSettings();
  void updateActionLayout();
  void resetActionBaseline();
  void persist();
  [[nodiscard]] std::string currentMainSceneName() const;

  CanvasService canvas_;
  OutputService outputs_;
  SettingsStore store_;
  ActionLayoutController actions_;
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
  QTimer *actionTimer_ = nullptr;
  DedicatedPreviewDock *dedicatedPreview_ = nullptr;
  QCheckBox *actionsEnabledCheck_ = nullptr;
  QComboBox *microphoneCombo_ = nullptr;
  QComboBox *webcamCombo_ = nullptr;
  QComboBox *chatCombo_ = nullptr;
  QComboBox *alertCombo_ = nullptr;
  QDoubleSpinBox *thresholdSpin_ = nullptr;
  QSpinBox *talkHoldSpin_ = nullptr;
  QSpinBox *talkScaleSpin_ = nullptr;
  QSpinBox *chatHoldSpin_ = nullptr;
  QSpinBox *alertHoldSpin_ = nullptr;
  QLabel *actionStatus_ = nullptr;
  QPushButton *cutsceneButton_ = nullptr;
  ActionLayoutState displayedActionState_ = ActionLayoutState::Normal;
  float animatedWebcamScale_ = 1.0f;
};

} // namespace dibu
