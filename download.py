import argparse
import os
import sys
import json

import torch
from transformers import AutoModelForCausalLM, AutoTokenizer, GenerationConfig, AutoConfig


# Directory utility
def verify_path(path):
    if not os.path.exists:
        os.makedirs(path)


def parse_args():
    parser = argparse.ArgumentParser()
    
    parser.add_argument('--model', type=str, default='8M', help='Model choice (default: 8M)')
    parser.add_argument('--model_dir', type=str, default='pretrained_models', help='Storage directory.')

    args = parser.parse_args()
    return args

def download_model(model_name, model_dir):
    # Create directory if it doesn't already exist
    verify_path(model_dir)

    model_name_formatted = 'TinyStories-' + model_name
    download_name = 'roneneldan/' + model_name_formatted

    model = AutoModelForCausalLM.from_pretrained(download_name)
    tokenizer = AutoTokenizer.from_pretrained(download_name)
    model.save_pretrained(os.path.join(model_dir, model_name_formatted), from_pt=True)
    tokenizer.save_pretrained(os.path.join(model_dir, model_name_formatted + '-tokenizer'))


if __name__ == "__main__":
    args = parse_args()
    dl = download_model(args.model, args.model_dir)