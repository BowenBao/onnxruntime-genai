// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// Modifications Copyright(C) 2024 Advanced Micro Devices, Inc. All rights reserved

#pragma once
#include "model.h"
#include "input_ids.h"
#include "logits.h"
#include "kv_cache.h"
#include "position_inputs.h"
#include "extra_inputs.h"

namespace Generators {

struct DecoderOnly_Model : Model {
  DecoderOnly_Model(std::unique_ptr<Config> config, OrtEnv& ort_env);

  std::unique_ptr<State> CreateState(RoamingArray<int32_t> sequence_lengths, const GeneratorParams& params) const override;

  std::unique_ptr<OrtSession> session_decoder_;
};

struct DecoderOnly_State : State {
  DecoderOnly_State(const DecoderOnly_Model& model, RoamingArray<int32_t> sequence_lengths, const GeneratorParams& params);
  RoamingArray<float> Run(int current_length, RoamingArray<int32_t> next_tokens, RoamingArray<int32_t> next_indices) override;
  const CapturedGraphInfo* GetCapturedGraphInfo() const override { return captured_graph_info_.get(); };

 protected:
  void UpdateInputsOutputs(const RoamingArray<int32_t>& next_tokens, RoamingArray<int32_t> next_indices, int current_length);

  const DecoderOnly_Model& model_;
  CapturedGraphInfoPtr captured_graph_info_;

  InputIDs input_ids_{model_, *this};
  Logits logits_{model_, *this};
  KV_Cache kv_cache_{model_, *this};
  PositionInputs position_inputs_;
  ExtraInputs extra_inputs_{model_, *this};
};

struct SpeculativeDecodingDecoderOnly_State : DecoderOnly_State {
  SpeculativeDecodingDecoderOnly_State(const DecoderOnly_Model& model, RoamingArray<int32_t> sequence_lengths, const GeneratorParams& params) : DecoderOnly_State{model, sequence_lengths, params} {};
  RoamingArray<float> Run(RoamingArray<int32_t> sequence, int next_token_length, int past_length, int return_last_logit_count) override;

 protected:
  void UpdateInputsOutputsFromSequence(const RoamingArray<int32_t>& sequence, size_t next_token_length, int past_length);
};

}  // namespace Generators
