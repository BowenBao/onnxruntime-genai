// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// Modifications Copyright(C) 2024 Advanced Micro Devices, Inc. All rights reserved

#pragma once

#include "static_buffer.h"

namespace Generators {

struct Logits {
  Logits(const Model& model, State& state);

  // Register input_ids as ORT session input.
  void Add();
  // For first iteration, find last token of each beam and store it in output_last_tokens_.
  // Also resizes logits to [bz, 1, vocab_size] for subsequent calls.
  RoamingArray<float> Get();
  // Retrieves logits[:, start:start + size, :].
  RoamingArray<float> Get(size_t start, size_t size);  // batch_size x size x vocab_size

  void Update();
  // Resize logits to [bz, token_count, vocab_size].
  void Update(size_t token_count);

 private:
  void HandleEOSArray(cpu_span<float> logits);

  const Model& model_;
  State& state_;
  size_t output_index_{~0U};

  std::array<int64_t, 3> shape_{};
  ONNXTensorElementDataType type_;

  // Tensor to keep the logits of the last tokens. It is used in the 2 cases below. Otherwhise, it is not used.
  // 1. prompt: store the last tokens logits from output_raw_
  // 2. token gen: store the converted fp32 logits if output_raw_ is fp16.
  std::unique_ptr<OrtValue> output_last_tokens_;

  std::unique_ptr<OrtValue> output_raw_;  // Raw logits output from model

  // Used for decoding runs with cuda graphs.
  StaticBuffer* sb_logits32_{};
  StaticBuffer* sb_logits16_{};

#if USE_CUDA
  cuda_unique_ptr<int32_t> cuda_eos_token_ids_ptr_;  // eos_token_ids from params, but in cuda accessible memory
  gpu_span<int32_t> cuda_eos_token_ids_;
#endif

#if USE_DML
  DmlReusedCommandListState logits_cast_command_list_state_{};
  std::unique_ptr<OrtValue> value32_cpu_;
#endif
};

}  // namespace Generators
