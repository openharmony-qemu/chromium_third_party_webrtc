/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "third_party/webrtc/modules/desktop_capture/ohos/base_screen_capture_source_bridge.h"

#include <ctime>

#include "base/command_line.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/task_runner.h"
#include "chrome/common/chrome_switches.h"

#include "modules/desktop_capture/ohos/audio_capture_info_adapter_impl.h"
#include "modules/desktop_capture/ohos/audio_enc_info_adapter_impl.h"
#include "modules/desktop_capture/ohos/audio_info_adapter_impl.h"
#include "modules/desktop_capture/ohos/recorder_info_adapter_impl.h"
#include "modules/desktop_capture/ohos/screen_capture_config_adapter_impl.h"
#include "modules/desktop_capture/ohos/video_capture_info_adapter_impl.h"
#include "modules/desktop_capture/ohos/video_enc_info_adapter_impl.h"
#include "modules/desktop_capture/ohos/video_info_adapter_impl.h"

namespace webrtc {
namespace {
constexpr int32_t kAudioSampleRate = 48000;
constexpr int32_t kAudioChannels = 2;
constexpr int32_t kInnerAudioSampleRate = 48000;
constexpr int32_t kInnerAudioBitrate = 48000;
}

class OHOSScreenCaptureCallback
    : public OHOS::NWeb::ScreenCaptureCallbackAdapter {
 public:
  OHOSScreenCaptureCallback(
      const scoped_refptr<base::SingleThreadTaskRunner>& task_runner,
      base::WeakPtr<BaseScreenCaptureSource> capturer)
      : task_runner_(task_runner), capturer_(capturer) {
    DCHECK(task_runner_.get());
    DCHECK(capturer_);
  }

  OHOSScreenCaptureCallback(const OHOSScreenCaptureCallback&) = delete;
  OHOSScreenCaptureCallback& operator=(const OHOSScreenCaptureCallback&) =
      delete;

  virtual ~OHOSScreenCaptureCallback() {}

  void OnError(int32_t errorCode) override {
    LOG(ERROR) << "OnError errorCode: " << errorCode;
    return;
  }

  void OnAudioBufferAvailable(
      bool isReady,
      OHOS::NWeb::AudioCaptureSourceTypeAdapter type) override {
    if (!isReady) {
      LOG(ERROR) << "OnVideoBufferAvailable isReady is false";
      return;
    }
    if (!BaseScreenCaptureSource::GetInstance().audio_callback_) {
      LOG(ERROR) << "BaseScreenCaptureSource audio callback is nullptr";
      return;
    }

    BaseScreenCaptureSource::GetInstance().audio_callback_->OnReadData(type);
  }

  void OnVideoBufferAvailable(bool isReady) override {
    if (!isReady) {
      LOG(ERROR) << "OnVideoBufferAvailable isReady is false";
      return;
    }
    if (!BaseScreenCaptureSource::GetInstance().window_callback_) {
      LOG(ERROR) << "BaseScreenCaptureSource window callback is nullptr";
      return;
    }

    BaseScreenCaptureSource::GetInstance().window_callback_->OnReadData();
  }

  void OnStateChange(OHOS::NWeb::ScreenCaptureStateCodeAdapter stateCode) override {
    task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&BaseScreenCaptureSource::SetScreenCaptureState, capturer_, stateCode));
  }

 private:
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
  base::WeakPtr<BaseScreenCaptureSource> capturer_;
};

  BaseScreenCaptureSource::BaseScreenCaptureSource()
  {
    LOG(INFO) << "BaseScreenCaptureSource::~BaseScreenCaptureSource";
  }

  BaseScreenCaptureSource::~BaseScreenCaptureSource()
  {
    LOG(INFO) << "BaseScreenCaptureSource::~BaseScreenCaptureSource";
  }

  int32_t BaseScreenCaptureSource::RegisterAudioCaptureCallback(std::shared_ptr<BaseScreenCaptureReadCallback> callbck) {
    LOG(INFO) << "BaseScreenCaptureSource::RegisterAduioCaptureCallback start";
    if (!callbck) {
      LOG(ERROR) << "register audio callback is nullptr";
      return -1;
    }
    audio_callback_ = callbck;
    return 0;
  }

  int32_t BaseScreenCaptureSource::RegisterWindowCaptureCallback(std::shared_ptr<BaseScreenCaptureReadCallback> callbck) {
    LOG(INFO) << "BaseScreenCaptureSource::RegisterWindowCaptureCallback start";
    if (!callbck) {
      LOG(ERROR) << "register window callback is nullptr";
      return -1;
    }
    window_callback_ = callbck;
    return 0;
  }

  bool BaseScreenCaptureSource::SetScreenCaptureConfig()
  {
      main_task_runner_ = base::SingleThreadTaskRunner::GetCurrentDefault();
      if (!main_task_runner_) {
          LOG(ERROR) << "get task runner failed";
          return false;
      }

      screen_capture_adapter_ =
        OHOS::NWeb::OhosAdapterHelper::GetInstance().CreateScreenCaptureAdapter();
      if (!screen_capture_adapter_) {
        LOG(ERROR) << "create screen capture adapter failed";
        return false;
      }

      auto displayMgr =
          OHOS::NWeb::OhosAdapterHelper::GetInstance().CreateDisplayMgrAdapter();
      if (!displayMgr) {
          LOG(ERROR) << "create display manager Adapter failed";
          return false;
      }
      auto display = displayMgr->GetDefaultDisplay();
      if (!display) {
          LOG(ERROR) << "display manager GetDefaultDisplay failed";
          return false;
      }
      int32_t videoFrameWidth = display->GetWidth();
      int32_t videoFrameHeight = display->GetHeight();
      LOG(INFO) << "screen capture videoFrameWidth: " << videoFrameWidth
                << "; videoFrameHeight: " << videoFrameHeight;

      //setting the microphone information
      std::shared_ptr<OHOS::NWeb::AudioCaptureInfoAdapterImpl> micCapInfo =
          std::make_shared<OHOS::NWeb::AudioCaptureInfoAdapterImpl>();
      micCapInfo->SetAudioSampleRate(kAudioSampleRate);
      micCapInfo->SetAudioChannels(kAudioChannels);
      micCapInfo->SetAudioSource(
          OHOS::NWeb::AudioCaptureSourceTypeAdapter::SOURCE_DEFAULT);

      //setting the system audio information
      std::shared_ptr<OHOS::NWeb::AudioCaptureInfoAdapterImpl> innerCapInfo =
          std::make_shared<OHOS::NWeb::AudioCaptureInfoAdapterImpl>();
      innerCapInfo->SetAudioSampleRate(kInnerAudioSampleRate);
      innerCapInfo->SetAudioChannels(kAudioChannels);
      innerCapInfo->SetAudioSource(
          OHOS::NWeb::AudioCaptureSourceTypeAdapter::ALL_PLAYBACK);

      //setting Audio Encoding Information
      std::shared_ptr<OHOS::NWeb::AudioEncInfoAdapterImpl> audioEncInfo =
          std::make_shared<OHOS::NWeb::AudioEncInfoAdapterImpl>();
      audioEncInfo->SetAudioBitrate(kInnerAudioBitrate);
      audioEncInfo->SetAudioCodecformat(
          OHOS::NWeb::AudioCodecFormatAdapter::AUDIO_DEFAULT);

      std::shared_ptr<OHOS::NWeb::AudioInfoAdapterImpl> audioInfo =
          std::make_shared<OHOS::NWeb::AudioInfoAdapterImpl>();
      audioInfo->SetMicCapInfo(micCapInfo);
      audioInfo->SetInnerCapInfo(innerCapInfo);
      audioInfo->SetAudioEncInfo(audioEncInfo);

      //setting video information
      std::shared_ptr<OHOS::NWeb::VideoCaptureInfoAdapterImpl> videoCapInfo =
          std::make_shared<OHOS::NWeb::VideoCaptureInfoAdapterImpl>();
      videoCapInfo->SetVideoFrameWidth(videoFrameWidth);
      videoCapInfo->SetVideoFrameHeight(videoFrameHeight);
      videoCapInfo->SetVideoSourceType(
          OHOS::NWeb::VideoSourceTypeAdapter::VIDEO_SOURCE_SURFACE_RGBA);

      //setting Video Encoding Information
      std::shared_ptr<OHOS::NWeb::VideoEncInfoAdapterImpl> videoEncInfo =
          std::make_shared<OHOS::NWeb::VideoEncInfoAdapterImpl>();
      videoEncInfo->SetVideoBitrate(2000000);
      videoEncInfo->SetVideoFrameRate(30);
      videoEncInfo->SetVideoCodecFormat(
          OHOS::NWeb::VideoCodecFormatAdapter::H264);

      std::shared_ptr<OHOS::NWeb::VideoInfoAdapterImpl> videoInfo =
          std::make_shared<OHOS::NWeb::VideoInfoAdapterImpl>();
      videoInfo->SetVideoCapInfo(videoCapInfo);

      std::shared_ptr<OHOS::NWeb::ScreenCaptureConfigAdapterImpl> config =
          std::make_shared<OHOS::NWeb::ScreenCaptureConfigAdapterImpl>();
      config->SetCaptureMode(OHOS::NWeb::CaptureModeAdapter::CAPTURE_HOME_SCREEN);
      config->SetDataType(OHOS::NWeb::DataTypeAdapter::ORIGINAL_STREAM_DATA_TYPE);
      config->SetAudioInfo(audioInfo);
      config->SetVideoInfo(videoInfo);

      if (screen_capture_adapter_->Init(config) != 0) {
          screen_capture_adapter_ = nullptr;
          LOG(ERROR) << "screen capture init failed";
          return false;
      }
      screen_capture_adapter_->SetMicrophoneEnable(false);
      auto callback = std::make_shared<OHOSScreenCaptureCallback>(
          main_task_runner_, weak_factory_.GetWeakPtr());
      if (screen_capture_adapter_->SetCaptureCallback(callback) != 0) {
          screen_capture_adapter_ = nullptr;
          LOG(ERROR) << "screen capture set capture callback failed";
          return false;
      }

      LOG(INFO) << "BaseScreenCaptureSource Init Success";
      return true;
  }

  void BaseScreenCaptureSource::SetScreenCaptureState(const OHOS::NWeb::ScreenCaptureStateCodeAdapter& stateCode) {
    capture_state_code_ = stateCode;
  }

  int32_t BaseScreenCaptureSource::StopCapture() {
    LOG(INFO) << "BaseScreenCaptureSource Stop Capture";
    int32_t ret = -1;
    if (screen_capture_adapter_) {
      ret = screen_capture_adapter_->StopCapture();
    }
    return ret;
  }

  int32_t BaseScreenCaptureSource::StartCapture() {
    LOG(INFO) << "BaseScreenCaptureSource Start Capture";
    int32_t ret = -1;
    if (screen_capture_adapter_) {
      ret = screen_capture_adapter_->StartCapture();
    }
    return ret;
  }

} // webrtc
