/*
 * Copyright (c) 2023-2025 Haitai FangYuan Co., Ltd.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MODULES_DESKTOP_CAPTURE_OHOS_CAPTURE_FRAME_H_
#define MODULES_DESKTOP_CAPTURE_OHOS_CAPTURE_FRAME_H_

#include <memory>
#include <vector>

#include "modules/desktop_capture/desktop_capture_types.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_geometry.h"
#include "modules/desktop_capture/resolution_tracker.h"
#include "modules/desktop_capture/shared_desktop_frame.h"
#include "modules/desktop_capture/shared_memory.h"

namespace webrtc {

class DesktopCaptureFrame final {
 public:
  // DesktopCaptureFrame does not take ownership of `factory`, consumers should
  // ensure it outlives this instance. nullptr is acceptable.
  explicit DesktopCaptureFrame(SharedMemoryFactory* factory);
  ~DesktopCaptureFrame();

  // Should not be called if Prepare() is not executed or returns false.
  SharedDesktopFrame* frame() const;

  // Prepares current instance with desktop size.
  bool Prepare(DesktopSize size);

 private:
  SharedMemoryFactory* const factory_;
  ResolutionTracker resolution_tracker_;
  std::unique_ptr<SharedDesktopFrame> frame_;
};

}  // namespace webrtc

#endif  // MODULES_DESKTOP_CAPTURE_OHOS_CAPTURE_FRAME_H_
