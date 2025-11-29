/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
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

#include "modules/desktop_capture/ohos/screenshot_desktop_capturer.h"

#include "base/logging.h"
#include "base/numerics/checked_math.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/ohos/desktop_capture_utils.h"
#include "ohos/adapter/xcomponent/adapter/window_adapter.h"

namespace webrtc {

ScreenshotDesktopCapturer::~ScreenshotDesktopCapturer() {
  if (pixelmap_ != nullptr) {
    Image_ErrorCode image_result = OH_PixelmapNative_Release(pixelmap_);
    if (image_result != IMAGE_SUCCESS) {
      LOG(WARNING) << __func__ << " [WebRTC] OH_PixelmapNative_Release failed: "
                   << static_cast<int>(image_result);
    }
  }
}

void ScreenshotDesktopCapturer::Init(const DesktopCaptureOptions& options) {
  options_ = options;
}

void ScreenshotDesktopCapturer::Start(Callback* callback) {
  RTC_DCHECK(!callback_);
  RTC_DCHECK(callback);
  callback_ = callback;
}

void ScreenshotDesktopCapturer::CaptureFrame() {
  NativeDisplayManager_ErrorCode result =
      OH_NativeDisplayManager_CaptureScreenPixelmap(display_id_, &pixelmap_);
  if (result != DISPLAY_MANAGER_OK || pixelmap_ == nullptr) {
    LOG(ERROR)
        << __func__
        << " [WebRTC] OH_NativeDisplayManager_CaptureScreenPixelmap failed: "
        << static_cast<int>(result);
    callback_->OnCaptureResult(Result::ERROR_PERMANENT, nullptr);
    return;
  }

  uint32_t image_width = 0;
  uint32_t image_height = 0;
  uint32_t image_stride = 0;
  if (!ReadOHPixelmap(pixelmap_, image_width, image_height, image_stride)) {
    callback_->OnCaptureResult(Result::ERROR_PERMANENT, nullptr);
    return;
  }

  auto checked_size =
      base::CheckMul(base::CheckMul(image_width, image_height), 4);
  size_t map_size;
  if (!checked_size.AssignIfValid(&map_size) || map_size == 0) {
    LOG(ERROR) << __func__ << " [WebRTC] Invalid map_size";
    callback_->OnCaptureResult(Result::ERROR_PERMANENT, nullptr);
    return;
  }

  std::vector<uint8_t> destination(map_size);
  uint8_t* src_data = destination.data();
  Image_ErrorCode image_result =
      OH_PixelmapNative_ReadPixels(pixelmap_, src_data, &map_size);
  if (image_result != IMAGE_SUCCESS || !src_data) {
    LOG(ERROR) << __func__ << " [WebRTC] OH_PixelmapNative_ReadPixels failed: "
               << static_cast<int>(image_result);
    callback_->OnCaptureResult(Result::ERROR_PERMANENT, nullptr);
    return;
  }

  webrtc::DesktopSize size(image_width, image_height);
  std::unique_ptr<DesktopFrame> frame =
      std::make_unique<BasicDesktopFrame>(size);
  frame->CopyPixelsFrom(src_data, image_stride,
                        DesktopRect::MakeSize(frame->size()));
  // The format of frame captured by OH is RGBA
  RGBAtoBGRAportable(reinterpret_cast<uint32_t*>(frame->data()),
                     reinterpret_cast<uint32_t*>(frame->data()),
                     frame->size().width() * frame->size().height());
  frame->mutable_updated_region()->SetRect(
      DesktopRect::MakeSize(frame->size()));

  callback_->OnCaptureResult(Result::SUCCESS, std::move(frame));
}

bool ScreenshotDesktopCapturer::ReadOHPixelmap(OH_PixelmapNative* pixelmap,
                                               uint32_t& image_width,
                                               uint32_t& image_height,
                                               uint32_t& image_stride) {
  OH_Pixelmap_ImageInfo* image_info = nullptr;
  Image_ErrorCode image_result = OH_PixelmapImageInfo_Create(&image_info);
  if (image_result != IMAGE_SUCCESS) {
    LOG(ERROR) << __func__ << " [WebRTC] OH_PixelmapImageInfo_Create failed: "
               << static_cast<int>(image_result);
    return false;
  }

  image_result = OH_PixelmapNative_GetImageInfo(pixelmap, image_info);
  if (image_result != IMAGE_SUCCESS || image_info == nullptr) {
    LOG(ERROR) << __func__
               << " [WebRTC] OH_PixelmapNative_GetImageInfo failed: "
               << static_cast<int>(image_result);
    return false;
  }

  image_result = OH_PixelmapImageInfo_GetWidth(image_info, &image_width);
  if (image_result != IMAGE_SUCCESS) {
    LOG(ERROR) << __func__ << " [WebRTC] OH_PixelmapImageInfo_GetWidth failed: "
               << static_cast<int>(image_result);
    return false;
  }

  image_result = OH_PixelmapImageInfo_GetHeight(image_info, &image_height);
  if (image_result != IMAGE_SUCCESS) {
    LOG(ERROR) << __func__
               << " [WebRTC] OH_PixelmapImageInfo_GetHeight failed: "
               << static_cast<int>(image_result);
    return false;
  }

  image_result = OH_PixelmapImageInfo_GetRowStride(image_info, &image_stride);
  if (image_result != IMAGE_SUCCESS) {
    LOG(ERROR) << __func__
               << " [WebRTC] OH_PixelmapImageInfo_GetRowStride failed: "
               << static_cast<int>(image_result);
    return false;
  }
  return true;
}

bool ScreenshotDesktopCapturer::GetSourceList(SourceList* sources) {
  RTC_DCHECK(!sources);
  RTC_DCHECK(sources->size() == 0);
  return true;
}

bool ScreenshotDesktopCapturer::SelectSource(SourceId id) {
  if (id < 0) {
    LOG(ERROR) << __func__ << " [WebRTC] Invalid source id: " << id;
    return false;
  }
  display_id_ = static_cast<uint32_t>(id);
  return true;
}

// static
std::unique_ptr<DesktopCapturer> ScreenshotDesktopCapturer::CreateRawCapturer(
    const DesktopCaptureOptions& options) {
  std::unique_ptr<ScreenshotDesktopCapturer> capturer =
      std::make_unique<ScreenshotDesktopCapturer>();
  capturer.get()->Init(options);
  return std::move(capturer);
}

}  // namespace webrtc
