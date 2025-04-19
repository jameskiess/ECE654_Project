import argparse
import os
import sys
import json

import torch
from transformers import AutoModelForCausalLM, AutoTokenizer, GenerationConfig


def parse_args():
    parser = argparse.ArgumentParser()
    
    parser.add_argument('--model_path', type=str, default='pretrained_models/TinyStories-8M', 
                        help='Model path (default: pretrained_models/TinyStories-8M)')
    parser.add_argument('--prompt', type=str, help = 'Prompt for inference.')

    args = parser.parse_args()
    return args


if __name__ == "__main__":
    args = parse_args()
    
    # Load model + tokenizer
    model = AutoModelForCausalLM.from_pretrained(args.model_path)
    tokenizer = AutoTokenizer.from_pretrained(args.model_path + '-tokenizer')
    model.eval()

    # Run inference
    input_ids = tokenizer(args.prompt, return_tensors="pt")
    output = model.generate(**input_ids, max_length=1000, num_beams=1)
    text_out = tokenizer.decode(output[0], skip_special_tokens=True)

    # Generate some model info
    total_param = 0
    param_emb = 0
    for name, param in model.named_parameters():
        print(f'Layer name: {name} -- # params: {param.numel()}, dimension = {param.shape}')
        total_param += param.numel()
        if 'wte' in name or 'wpe' in name:
            param_emb += param.numel()
    print(f'Total number of parameters: {total_param}')
    print(f'Total number of parameters after embedding: {total_param - param_emb}\n')

    lm_head = model.get_output_embeddings()
    print(f'LM classification head: {lm_head}')
    print(f'LM classification head weights: {lm_head.weight.shape}\n')

    print(f'Input IDs (tokens): {input_ids}\n')

    print('Prompt: ' + args.prompt)
    print('Response: ' + text_out)