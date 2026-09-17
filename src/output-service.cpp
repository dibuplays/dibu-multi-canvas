#include "output-service.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

namespace dibu {

OutputService::~OutputService()
{
  shutdown();
}

obs_encoder_t *OutputService::cloneVideoEncoder(obs_encoder_t *source, obs_canvas_t *canvas, const char *name)
{
  const char *id = source ? obs_encoder_get_id(source) : "obs_x264";
  obs_data_t *settings = source ? obs_encoder_get_settings(source) : obs_data_create();
  obs_encoder_t *encoder = obs_video_encoder_create(id, name, settings, nullptr);
  obs_data_release(settings);
  if (encoder)
    obs_encoder_set_video(encoder, obs_canvas_get_video(canvas));
  return encoder;
}

obs_encoder_t *OutputService::cloneAudioEncoder(obs_encoder_t *source, const char *name)
{
  const char *id = source ? obs_encoder_get_id(source) : "ffmpeg_aac";
  obs_data_t *settings = source ? obs_encoder_get_settings(source) : obs_data_create();
  obs_encoder_t *encoder = obs_audio_encoder_create(id, name, settings, 0, nullptr);
  obs_data_release(settings);
  if (encoder)
    obs_encoder_set_audio(encoder, obs_get_audio());
  return encoder;
}

void OutputService::setOutputError(obs_output_t *output, const char *fallback)
{
  const char *error = output ? obs_output_get_last_error(output) : nullptr;
  lastError_ = error && *error ? error : fallback;
}

bool OutputService::startRecording(obs_canvas_t *canvas)
{
  if (!canvas || recording())
    return recording();
  releaseRecording();
  lastError_.clear();

  obs_output_t *mainOutput = obs_frontend_get_recording_output();
  obs_encoder_t *mainVideo = mainOutput ? obs_output_get_video_encoder(mainOutput) : nullptr;
  obs_encoder_t *mainAudio = mainOutput ? obs_output_get_audio_encoder(mainOutput, 0) : nullptr;

  recordingVideo_ = cloneVideoEncoder(mainVideo, canvas, "Dibu Vertical Recording Video");
  recordingAudio_ = cloneAudioEncoder(mainAudio, "Dibu Vertical Recording Audio");

  obs_data_t *settings = obs_data_create();

  QString directory = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
  if (directory.isEmpty())
    directory = QDir::homePath();
  directory += QStringLiteral("/Dibu Multi-Canvas");
  QDir().mkpath(directory);
  const QString filename = QStringLiteral("Dibu-Vertical-%1.mkv")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss")));
  recordingPath_ = QDir(directory).filePath(filename).toStdString();
  obs_data_set_string(settings, "path", recordingPath_.c_str());

  recordingOutput_ = obs_output_create("ffmpeg_muxer", "Dibu Vertical Recording", settings, nullptr);
  obs_data_release(settings);
  if (mainOutput)
    obs_output_release(mainOutput);

  if (!recordingOutput_ || !recordingVideo_ || !recordingAudio_) {
    setOutputError(recordingOutput_, "Could not create the vertical recording output or encoders.");
    releaseRecording();
    return false;
  }

  obs_output_set_video_encoder(recordingOutput_, recordingVideo_);
  obs_output_set_audio_encoder(recordingOutput_, recordingAudio_, 0);
  if (!obs_output_start(recordingOutput_)) {
    setOutputError(recordingOutput_, "OBS could not start vertical recording.");
    releaseRecording();
    return false;
  }
  return true;
}

void OutputService::stopRecording()
{
  if (recordingOutput_ && obs_output_active(recordingOutput_))
    obs_output_stop(recordingOutput_);
}

bool OutputService::recording() const
{
  return recordingOutput_ && obs_output_active(recordingOutput_);
}

bool OutputService::startStreaming(obs_canvas_t *canvas, const std::string &server, const std::string &key)
{
  if (!canvas || server.empty() || key.empty()) {
    lastError_ = "Enter both the RTMP server URL and stream key.";
    return false;
  }
  if (streaming())
    return true;
  releaseStreaming();
  lastError_.clear();

  obs_output_t *mainOutput = obs_frontend_get_streaming_output();
  obs_encoder_t *mainVideo = mainOutput ? obs_output_get_video_encoder(mainOutput) : nullptr;
  obs_encoder_t *mainAudio = mainOutput ? obs_output_get_audio_encoder(mainOutput, 0) : nullptr;
  streamingVideo_ = cloneVideoEncoder(mainVideo, canvas, "Dibu Vertical Streaming Video");
  streamingAudio_ = cloneAudioEncoder(mainAudio, "Dibu Vertical Streaming Audio");

  obs_data_t *serviceSettings = obs_data_create();
  obs_data_set_string(serviceSettings, "server", server.c_str());
  obs_data_set_string(serviceSettings, "key", key.c_str());
  streamingService_ = obs_service_create("rtmp_custom", "Dibu Vertical RTMP Service", serviceSettings, nullptr);
  obs_data_release(serviceSettings);

  obs_data_t *outputSettings = obs_data_create();
  streamingOutput_ = obs_output_create("rtmp_output", "Dibu Vertical Stream", outputSettings, nullptr);
  obs_data_release(outputSettings);
  if (mainOutput)
    obs_output_release(mainOutput);

  if (!streamingOutput_ || !streamingService_ || !streamingVideo_ || !streamingAudio_) {
    setOutputError(streamingOutput_, "Could not create the vertical streaming output.");
    releaseStreaming();
    return false;
  }

  obs_output_set_service(streamingOutput_, streamingService_);
  obs_output_set_video_encoder(streamingOutput_, streamingVideo_);
  obs_output_set_audio_encoder(streamingOutput_, streamingAudio_, 0);
  if (!obs_output_start(streamingOutput_)) {
    setOutputError(streamingOutput_, "OBS could not start the vertical stream.");
    releaseStreaming();
    return false;
  }
  return true;
}

void OutputService::stopStreaming()
{
  if (streamingOutput_ && obs_output_active(streamingOutput_))
    obs_output_stop(streamingOutput_);
}

bool OutputService::streaming() const
{
  return streamingOutput_ && obs_output_active(streamingOutput_);
}

void OutputService::releaseRecording()
{
  if (recordingOutput_) {
    if (obs_output_active(recordingOutput_))
      obs_output_stop(recordingOutput_);
    obs_output_release(recordingOutput_);
    recordingOutput_ = nullptr;
  }
  if (recordingVideo_) {
    obs_encoder_release(recordingVideo_);
    recordingVideo_ = nullptr;
  }
  if (recordingAudio_) {
    obs_encoder_release(recordingAudio_);
    recordingAudio_ = nullptr;
  }
}

void OutputService::releaseStreaming()
{
  if (streamingOutput_) {
    if (obs_output_active(streamingOutput_))
      obs_output_stop(streamingOutput_);
    obs_output_release(streamingOutput_);
    streamingOutput_ = nullptr;
  }
  if (streamingVideo_) {
    obs_encoder_release(streamingVideo_);
    streamingVideo_ = nullptr;
  }
  if (streamingAudio_) {
    obs_encoder_release(streamingAudio_);
    streamingAudio_ = nullptr;
  }
  if (streamingService_) {
    obs_service_release(streamingService_);
    streamingService_ = nullptr;
  }
}

void OutputService::shutdown()
{
  releaseRecording();
  releaseStreaming();
}

} // namespace dibu
