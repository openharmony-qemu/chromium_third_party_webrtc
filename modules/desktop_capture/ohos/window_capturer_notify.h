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

#ifndef MODULES_DESKTOP_CAPTURE_OHOS_WINDOW_CAPTURER_NOTIFY_H_
#define MODULES_DESKTOP_CAPTURE_OHOS_WINDOW_CAPTURER_NOTIFY_H_

#include <memory>
#include <unordered_map>

#include "modules/desktop_capture/ohos/capturer_event_callback.h"

namespace webrtc {

class hash_ {
 public:
  size_t operator()(OH_AVScreenCapture* val) const {
    return std::hash<unsigned long>()(reinterpret_cast<unsigned long>(val));
  }
};

class equal_ {
 public:
  bool operator()(OH_AVScreenCapture* val1, OH_AVScreenCapture* val2) const {
    return val1 == val2;
  }
};

class WindowCapturerNotify {
 public:
  WindowCapturerNotify();
  ~WindowCapturerNotify();
  WindowCapturerNotify(WindowCapturerNotify& other) = delete;
  void operator=(const WindowCapturerNotify&) = delete;

  static WindowCapturerNotify* GetInstance();

  CapturerEventCallBack* GetEventCallBack(OH_AVScreenCapture* capture);
  bool RegisterEventCallback(OH_AVScreenCapture* capture,
                             std::shared_ptr<CapturerEventCallBack> callback,
                             void* userData);
  void UnRegisterEventCallback(OH_AVScreenCapture* capture);

 private:
  std::unordered_map<OH_AVScreenCapture*,
                     std::shared_ptr<CapturerEventCallBack>,
                     hash_,
                     equal_>
      callback_map_;
};
}  // namespace webrtc
#endif  // MODULES_DESKTOP_CAPTURE_OHOS_WINDOW_CAPTURER_NOTIFY_H_
