/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_DESKTOP_CAPTURE_OHOS_BASE_WINDOW_CAPTURER_H_
#define MODULES_DESKTOP_CAPTURE_OHOS_BASE_WINDOW_CAPTURER_H_

#include <memory>

#include "absl/types/optional.h"
#include "base/memory/weak_ptr.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_capture_options.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "ohos_adapter_helper.h"
#include "rtc_base/synchronization/mutex.h"
#include "media/base/audio_capturer_source.h"
#include "third_party/webrtc/modules/desktop_capture/ohos/base_screen_capture_read_callback.h"
#include "third_party/webrtc/modules/desktop_capture/ohos/base_screen_capture_source_bridge.h"

namespace webrtc {
class BaseScreenCaptureReadCallback;

using namespace OHOS::NWeb;
using namespace media;

using OnReadDataCallback = base::RepeatingCallback<void(void)>;

class WindowCapturerReadCallback : public BaseScreenCaptureReadCallback{
 public:
  WindowCapturerReadCallback(const OnReadDataCallback& readDataCallback);

  ~WindowCapturerReadCallback();

  void OnReadData() override;

  void OnReadData(OHOS::NWeb::AudioCaptureSourceTypeAdapter type) override {}

 private:
  OnReadDataCallback readDataCallback_;
};

class BaseWindowCapturer : public DesktopCapturer {
 public:
  enum class CaptureSourceType : uint32_t {
    kScreen = 0b01,
    kWindow = 0b10,
    kAny = 0b11
  };

  enum class CursorMode : uint32_t {
    kHidden = 0b01,
    kEmbedded = 0b10,
    kMetadata = 0b100
  };

  struct PipeWireVersion {
    int major = 0;
    int minor = 0;
    int micro = 0;
  };

  explicit BaseWindowCapturer(CaptureSourceType source_type);
  ~BaseWindowCapturer() override;

  static std::unique_ptr<DesktopCapturer> CreateRawCapturer(
      const DesktopCaptureOptions& options, const CaptureSourceType& type);

  // DesktopCapturer interface.
  void Start(Callback* delegate) override;
  void CaptureFrame() override;
  bool GetSourceList(SourceList* sources) override;
  bool SelectSource(SourceId id) override;

  void SetSharedMemoryFactory(std::unique_ptr<SharedMemoryFactory> shared_memory_factory) override;

  void HandleBuffer();

  void SetScreenCaptureState(const OHOS::NWeb::ScreenCaptureStateCodeAdapter& stateCode);

 private:
  static DesktopCapturer::Result HandleCaptureStateCode(const OHOS::NWeb::ScreenCaptureStateCodeAdapter& code);

  webrtc::Mutex current_frame_lock_;

  std::unique_ptr<DesktopFrame> current_frame_;

  OHOS::NWeb::ScreenCaptureStateCodeAdapter capture_state_code_
      = OHOS::NWeb::ScreenCaptureStateCodeAdapter::SCREEN_CAPTURE_STATE_INVLID;

  Callback* callback_ = nullptr;

  CaptureSourceType capture_source_type_ =
      BaseWindowCapturer::CaptureSourceType::kScreen;

  bool portal_init_failed_ = false;

  std::unique_ptr<SharedMemoryFactory> factory_ = nullptr;

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  base::WeakPtrFactory<BaseWindowCapturer> weak_factory_{this};

  bool isStart_ = false;

  SourceId selectSourceId_ = -1;

  std::shared_ptr<WindowCapturerReadCallback> WindowCapturerReadCallback_ = nullptr;
};

}  // namespace webrtc

#endif  // MODULES_DESKTOP_CAPTURE_OHOS_BASE_WINDOW_CAPTURER_H_
