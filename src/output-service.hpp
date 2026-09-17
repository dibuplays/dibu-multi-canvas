#pragma once

#include <string>

struct obs_canvas;
typedef struct obs_canvas obs_canvas_t;
struct obs_output;
typedef struct obs_output obs_output_t;
struct obs_encoder;
typedef struct obs_encoder obs_encoder_t;
struct obs_service;
typedef struct obs_service obs_service_t;

namespace dibu {

class OutputService {
public:
  OutputService() = default;
  ~OutputService();

  OutputService(const OutputService &) = delete;
  OutputService &operator=(const OutputService &) = delete;

  bool startRecording(obs_canvas_t *canvas);
  void stopRecording();
  [[nodiscard]] bool recording() const;
  [[nodiscard]] const std::string &recordingPath() const noexcept { return recordingPath_; }

  bool startStreaming(obs_canvas_t *canvas, const std::string &server, const std::string &key);
  void stopStreaming();
  [[nodiscard]] bool streaming() const;
  [[nodiscard]] const std::string &lastError() const noexcept { return lastError_; }

  void shutdown();

private:
  obs_encoder_t *cloneVideoEncoder(obs_encoder_t *source, obs_canvas_t *canvas, const char *name);
  obs_encoder_t *cloneAudioEncoder(obs_encoder_t *source, const char *name);
  void releaseRecording();
  void releaseStreaming();
  void setOutputError(obs_output_t *output, const char *fallback);

  obs_output_t *recordingOutput_ = nullptr;
  obs_encoder_t *recordingVideo_ = nullptr;
  obs_encoder_t *recordingAudio_ = nullptr;
  obs_output_t *streamingOutput_ = nullptr;
  obs_encoder_t *streamingVideo_ = nullptr;
  obs_encoder_t *streamingAudio_ = nullptr;
  obs_service_t *streamingService_ = nullptr;
  std::string recordingPath_;
  std::string lastError_;
};

} // namespace dibu
