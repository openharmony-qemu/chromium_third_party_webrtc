// Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_DESKTOP_CAPTURE_OHOS_BASE_WINDOW_CAPTURER_H_
#define MODULES_DESKTOP_CAPTURE_OHOS_BASE_WINDOW_CAPTURER_H_

#include <memory>

#include "absl/types/optional.h"
#include "base/memory/weak_ptr.h"
#include "base/task/single_thread_task_runner.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/ohos/desktop_capture_frame.h"
#include "modules/desktop_capture/ohos/native_frame.h"
#include "modules/desktop_capture/screen_capture_frame_queue.h"
#include "modules/desktop_capture/shared_desktop_frame.h"
#include "multimedia/player_framework/native_avscreen_capture.h"
#include "native_buffer/native_buffer.h"
#include "ohos/adapter/screen/screen_adapter.h"
#include "rtc_base/synchronization/mutex.h"

namespace webrtc {

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
      const DesktopCaptureOptions& options,
      const CaptureSourceType& type);

  bool Init(const DesktopCaptureOptions& options);

  // DesktopCapturer interface.
  void Start(Callback* delegate) override;
  void CaptureFrame() override;
  bool GetSourceList(SourceList* sources) override;
  bool SelectSource(SourceId id) override;

  void SetSharedMemoryFactory(
      std::unique_ptr<SharedMemoryFactory> shared_memory_factory) override;

  // OHOSScreenCaptureCallback interface.
  void HandleStateChange(OH_AVScreenCaptureStateCode stateCode, void* userData);
  void HandleError(int32_t errorCode, void* userData);
  void HandleBuffer(OH_AVBuffer* buffer,
                    OH_AVScreenCaptureBufferType bufferType,
                    int64_t timestamp,
                    void* userData);
  void StopCapture();
  base::WeakPtr<BaseWindowCapturer> GetWeakPtr();

 private:
  void DoCapture();
  void DoCaptureSync();
  bool EnsureFrameCaptured();
  void SetState(bool isStart) { is_start_ = isStart; }

  webrtc::Mutex current_frame_lock_;
  DesktopCaptureOptions options_;
  ScreenCaptureFrameQueue<DesktopCaptureFrame> queue_;
  Callback* callback_ = nullptr;
  CaptureSourceType capture_source_type_ =
      BaseWindowCapturer::CaptureSourceType::kScreen;
  std::unique_ptr<SharedMemoryFactory> shared_memory_factory_ = nullptr;
  OH_AVScreenCapture* screen_capture_ = nullptr;
  bool is_start_ = false;
  SourceId select_source_id_ = -1;
  ohos::adapter::OhosDisplay ohos_screen_;

  base::WeakPtrFactory<BaseWindowCapturer> weak_factory_{this};
};

}  // namespace webrtc

#endif  // MODULES_DESKTOP_CAPTURE_OHOS_BASE_WINDOW_CAPTURER_H_
