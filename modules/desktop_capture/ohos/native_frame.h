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

#ifndef MODULES_DESKTOP_CAPTURE_OHOS_NATIVE_FRAME_H_
#define MODULES_DESKTOP_CAPTURE_OHOS_NATIVE_FRAME_H_

#include <memory>

#include "multimedia/player_framework/native_avscreen_capture.h"
#include "native_buffer/native_buffer.h"

namespace webrtc {

class NativeFrame final {
 public:
  static std::unique_ptr<NativeFrame> Create(
      OH_AVScreenCapture* screen_capture);

  static std::unique_ptr<NativeFrame> Create(OH_AVBuffer* buffer);

  explicit NativeFrame(OH_AVScreenCapture* screen_capture);
  explicit NativeFrame(OH_AVBuffer* buffer);
  ~NativeFrame();

  OH_NativeBuffer* GetNativeBuffer() const;

  bool Init();
  uint8_t* Map();
  void UnMap();
  uint8_t* GetBuffer();
  int32_t GetCapacity();
  int32_t image_width() const { return config_.width; }
  int32_t image_height() const { return config_.height; }
  int32_t image_stride() const { return config_.stride; }

 private:
  bool InitWithScreenCapture();
  bool InitWithBuffer();
  OH_AVScreenCapture* screen_capture_ = nullptr;
  OH_AVBuffer* buffer_ = nullptr;
  OH_NativeBuffer* native_buffer_ = nullptr;
  OH_NativeBuffer_Config config_;
  uint8_t* src_data_ = nullptr;
};

}  // namespace webrtc

#endif  // MODULES_DESKTOP_CAPTURE_OHOS_NATIVE_FRAME_H_
