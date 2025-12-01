// Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/logging.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/ohos/base_window_capturer.h"
#include "modules/desktop_capture/ohos/screenshot_desktop_capturer.h"

namespace webrtc {

// static
std::unique_ptr<DesktopCapturer> DesktopCapturer::CreateRawScreenCapturer(
    const DesktopCaptureOptions& options) {
  LOG(INFO) << __func__ << " [DesktopCapturer] is createScreenshotDesktopCapturer: " << options.use_screenshot();
  if (options.use_screenshot()) {
    return ScreenshotDesktopCapturer::CreateRawCapturer(options);
  }

  return BaseWindowCapturer::CreateRawCapturer(
      options, BaseWindowCapturer::CaptureSourceType::kScreen);
}

}  // namespace webrtc
