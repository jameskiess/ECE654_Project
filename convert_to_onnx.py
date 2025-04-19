from pathlib import Path
import torch
import os
import transformers
from transformers import AutoModelForCausalLM, AutoTokenizer, GenerationConfig
from transformers.onnx import FeaturesManager


if __name__ == "__main__":
    model_path = './pretrained_models/TinyStories-3M'
    tokenizer_path = './pretrained_models/TinyStories-3M-tokenizer'
    onnx_path = './exported_models'
    onnx_fname = 'TinyStories-3M.onnx'

    feature = 'causal-lm'

    if not os.path.exists(onnx_path):
        os.makedirs(onnx_path)

    tokenizer = AutoTokenizer.from_pretrained(tokenizer_path)
    model = AutoModelForCausalLM.from_pretrained(model_path)

    # Load config
    model_kind, model_onnx_config = FeaturesManager.check_supported_model_or_raise(
        model, feature=feature
    )
    onnx_config = model_onnx_config(model.config)

    # Export
    onnx_inputs, onnx_outputs = transformers.onnx.export(
        preprocessor=tokenizer,
        model=model,
        config=onnx_config,
        opset=13,
        output=Path(onnx_path + '/' + onnx_fname)
    )