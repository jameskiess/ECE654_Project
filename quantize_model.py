import os
import re
import sys
import argparse
import copy
import torch
import math

from datasets import load_dataset

import modelopt.torch.opt as mto
import modelopt.torch.quantization as mtq
from modelopt.torch.export import export_hf_checkpoint
from modelopt.torch.utils.dataset_utils import get_max_batch_size
from modelopt.torch.quantization.calib.max import MaxCalibrator

from transformers import AutoModelForCausalLM, AutoTokenizer, GenerationConfig

device = 'cuda'

def get_int8_config(
        model,
        alpha=0.5
):
    quant_config = copy.deepcopy(mtq.INT8_SMOOTHQUANT_CFG)
    quant_config['algorithm'] = {'method': 'smoothquant', 'alpha': alpha}

    return quant_config


if __name__ == '__main__':
    # Hard-coded paths
    model_path = './pretrained_models/TinyStories-3M'
    tokenizer_path = './pretrained_models/TinyStories-3M-tokenizer'

    quant_path = f'./compressed_models/TinyStories-3M-w8a8'
    if not os.path.exists(quant_path):
        os.makedirs(quant_path)

    # Load model
    model = AutoModelForCausalLM.from_pretrained(model_path)
    tokenizer = AutoTokenizer.from_pretrained(tokenizer_path)
    model.eval()

    # Get calibration set by randomly sampling 256 training samples and
    # cutting them off at ~40% to use as a prompt (per original paper)
    ds = load_dataset('roneneldan/TinyStories', split='train')
    ds = ds[0:512]['text']

    prompts = []
    for s in ds:
        toks = s.split()
        prompts.append(" ".join(toks[0:math.floor(len(toks)*0.4)]))

    # Get quant configuration
    quant_config = get_int8_config(model)

    # Forward loop
    def forward_loop(model):
        for prompt in prompts:
            input_ids = tokenizer(prompt, return_tensors="pt")
            output = model.generate(**input_ids, max_length=1000, num_beams=1)
            text_out = tokenizer.decode(output[0], skip_special_tokens=True)

    model = mtq.quantize(model, quant_config, forward_loop=forward_loop)

    # For debugging
    print('>>> DEBUG <<< Printing quantization summary:')
    mtq.print_quant_summary(model)
    print('\n\n')

    # Save quantized model
    torch.save(mto.modelopt_state(model), quant_path + '/quant_states_qint8.pt')
    torch.save(model.state_dict(), quant_path + '/model_state_dict_qint8.pt')
    mto.save(model, quant_path + '/model_checkpoint')
    tokenizer.save_pretrained(quant_path)

    #export_hf_checkpoint(model, export_dir=quant_path)
    #model.save_quantized(quant_path)
    #tokenizer.save_pretrained(quant_path)
