import argparse
import os
import sys
import json
import math
import numpy as np

import torch
from transformers import AutoModelForCausalLM, AutoTokenizer, GenerationConfig

import modelopt.torch.opt as mto
import modelopt.torch.quantization as mtq

import matplotlib.pyplot as plt

from datasets import load_dataset
from rouge import Rouge

device = 'cuda'


def dump_json(fname, data):
    f = open(fname, 'w')
    json.dump(data, f, indent=4)
    f.close()


def loop_inference(prompts, model_q, model_fp, tokenizer):
    model_q.eval()
    model_fp.eval()

    resp = []
    for prompt in prompts:
        tmp = {}

        # Run inference
        input_ids_FP = tokenizer(prompt, return_tensors="pt")
        input_ids_Q = tokenizer(prompt, return_tensors="pt")

        output_FP = model_fp.generate(**input_ids_FP, max_length=1000, num_beams=1)
        text_FP = tokenizer.decode(output_FP[0], skip_special_tokens=True)

        output_Q = model_q.generate(**input_ids_Q, max_length=1000, num_beams=1)
        text_Q = tokenizer.decode(output_Q[0], skip_special_tokens=True)

        # Add to dict
        tmp['prompt'] = prompt
        tmp['FP_response'] = text_FP
        tmp['Quant_response'] = text_Q
        resp.append(tmp)

    return resp


def gen_hist(fname, data, title, xlabel, ylabel):
    data = np.array(data)

    fig = plt.figure(figsize=(12,8), facecolor='white')
    ax = fig.add_subplot(1,1,1, frameon=False)

    ax.set_title(title, fontsize=14)
    ax.set_xlabel(xlabel, fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)

    plt.hist(data)

    plt.grid(axis='y')
    fig.tight_layout()
    plt.savefig(fname)
    plt.close(fig)


if __name__ == '__main__':
    model_path = './pretrained_models/TinyStories-3M'
    tokenizer_path = './pretrained_models/TinyStories-3M-tokenizer'
    quant_path = './compressed_models/TinyStories-3M-w8a8'

    # Load FP and quantized models
    tokenizer = AutoTokenizer.from_pretrained(tokenizer_path)

    model_FP = AutoModelForCausalLM.from_pretrained(model_path)
    model_quant = AutoModelForCausalLM.from_pretrained(model_path)

    # Load quantization
    model_quant = mto.restore_from_modelopt_state(model_quant, torch.load(f'{quant_path}/quant_states_qint8.pt'))
    model_quant.load_state_dict(torch.load(f'{quant_path}/model_state_dict_qint8.pt'))

    model_FP.eval()
    model_quant.eval()

    # Perform evaluation:
    #   1) Qualitative analysis using Evaluation_prompts.yaml to match paper
    #      Due to time constraints, just select a few and manually evaluate
    #
    #   2) Quantitative analysis using Rouge score and 100 randomly-selected
    #      prompts from training dataset, as explained in paper. Note that we
    #      modify the evaluation slightly from the paper:
    #       a) Comparison of generation and original story - R_{2,p}(T_i, T'_i)
    #       b) Similarity of generated stories - max[R_{2}(T_i, T_j)]
    #
    #      https://pypi.org/project/rouge/
    #
    #   3) If time permits, perplexity analysis

    """
    # =================== 1) ============================
    factual_prompts = [
        'Alice was so tired when she got back home so she went',
        'Jack and Lily saw a rainbow after a rainy day. They were amazed by the colors. Jack said, \"Look, Lily. A rainbow has',
        'Jack and Lily liked to watch the moon at night. They noticed that the moon changed its shape every night. Sometimes the moon was big and round, and sometimes it was',
        'Jack wanted to read a book, so we went to',
        '\"Can cows fly?\", Alice asked her mother.',
        '\"What do birds lke to eat?\", Tom asked his mother.',
        '\"What language do they speak in France?\", Tom asked his mother',
        'If I throw a ball up in the air, eventually it will',
        'It was winter and cold outside so his mother told him, \"You should'
    ]

    reasoning_prompts = [
        'Lily likes cats and dogs. She asked her mom for a dog and her mom said no, so instead she asked',
        'Jack told Mary, \"If you give me your banana, I\'ll give you my apple\". Mary gave Jack her Banana so',
        'On weekends Jack went to visit his grandmother whereas on weekdays he would go to school. Last weekend, when Jack was on his way to',
        'Lily and Ben were having an argument. Ben said that cake is much better than ice cream and Lily said that',
        'Lily and Ben are having an argument. They are trying to decide between the park and the swimming pool. Ben says, \"I want to go to the park\". Lily says',
        'Jack\'s mother was not home, and his father was at home. When Jack came home, he said hello to',
        'Lily doesn\'t like swimming. When her father wants to take her to the swimming pool, she says',
        'Both Ben and Lily wanted cake. Father said that there was only one piece of cake left. They',
        'Ben went to visit Lily in her home, but she was not at home. Ben knocked on the door,'
    ]

    context_prompts = [
        '\"Hi Jane, have you seen Alice? I can\'t find her anywhere\", said Jack.',
        'Max had two dogs. One was white and the other was black. Max walked up the street and saw a kid with a dog. He told the kid, \"I see you have a Brown dog. I also have',
        'Anne had a piece of candy in her left pocket and a piece of chocolate in her right pocket. Anne\'s mom asked her, \"Anne, what is that you have in your left pocket?\"',
        'Alice had both an apple and a carrot in her bag. She took the apple out of the bag and gave it to Jack. She reached into the bag again and took',
        'Alice and Jack walked up the street and met a girl in a red dress. The girl said to them, \"Hi, I\'m Jane. What are your names?\"',
        'Diva was hungry, and wanted to bake a cake, but she didn\'t have any sugar at home, so she decided to go ask around. She started walking and met a squirrel. She asked the squirrel, \"Would you happen'
    ]

    # Factual responses
    fact_resp = loop_inference(factual_prompts, model_quant, model_FP, tokenizer)
    dump_json('factual_responses.json', fact_resp)

    # Reasoning responses
    reason_resp = loop_inference(reasoning_prompts, model_quant, model_FP, tokenizer)
    dump_json('reasoning_responses.json', reason_resp)

    # Contextual responses
    context_resp = loop_inference(context_prompts, model_quant, model_FP, tokenizer)
    dump_json('contextual_responses.json', context_resp)
    """

    # ================== 2) =========================
    # Randomly sample 100 training samples
    ds = load_dataset('roneneldan/TinyStories', split='validation')
    ds = ds[0:100]['text']

    prompts = []
    residuals = []
    prompt_indices = []
    for s in ds:
        toks = s.split()
        prompts.append(" ".join(toks[0:math.floor(len(toks)*0.4)]))
        residuals.append(" ".join(toks[math.floor(len(toks)*0.4):]))
        prompt_indices.append(math.floor(len(toks)*0.4))

    # Collect responses from FP and quantized model
    pscores_fp = []
    pscores_q = []

    max_fscores_fp = []
    max_fscores_q = []

    fscores = []

    rouge = Rouge()
    rouge_responses = loop_inference(prompts, model_quant, model_FP, tokenizer)
    for i in range(len(rouge_responses)):
        maxf_fp = float('-inf')
        maxf_q = float('-inf')

        fp_resp = rouge_responses[i]['FP_response']
        q_resp = rouge_responses[i]['Quant_response']
        orig_text = residuals[i]
        #orig_text = ds[i]

        # Trim responses to look only at generated text (ignore prompts)
        fp_resp = " ".join(fp_resp.split()[prompt_indices[i]:])
        q_resp = " ".join(q_resp.split()[prompt_indices[i]:])

        # Precision score between generations and original
        pscores_fp.append(rouge.get_scores(fp_resp, orig_text)[0]['rouge-2']['p'])
        pscores_q.append(rouge.get_scores(q_resp, orig_text)[0]['rouge-2']['p'])

        # Maximum Fmeasure similarity within generated stories
        for j in range(len(rouge_responses)):
            if i != j:
                fp_resp_j = rouge_responses[j]['FP_response']
                q_resp_j = rouge_responses[j]['Quant_response']

                fp_resp_j = " ".join(fp_resp_j.split()[prompt_indices[j]:])
                q_resp_j = " ".join(q_resp_j.split()[prompt_indices[j]:])

                fscore_fp = rouge.get_scores(fp_resp, fp_resp_j)[0]['rouge-2']['f']
                fscore_q = rouge.get_scores(q_resp, q_resp_j)[0]['rouge-2']['f']

                if fscore_fp >= maxf_fp:
                    maxf_fp = fscore_fp
                if fscore_q >= maxf_q:
                    maxf_q = fscore_q

        max_fscores_fp.append(maxf_fp)
        max_fscores_q.append(maxf_q)

        # F score between quantized and FP model
        fscores.append(rouge.get_scores(q_resp, fp_resp)[0]['rouge-2']['f'])
                
    # Generate and save histograms
    gen_hist(fname = 'hist_pscore_fp.png', data = pscores_fp,
             title = 'Rouge2 Precision Score vs Original (Full Precision)', 
             xlabel = 'P score', ylabel = 'Frequency')

    gen_hist(fname = 'hist_pscore_quant.png', data = pscores_q,
             title = 'Rouge2 Precision Score vs Original (Quantized)', 
             xlabel = 'P score', ylabel = 'Frequency')

    gen_hist(fname = 'hist_max_fscore_fp.png', data = max_fscores_fp,
             title = 'Maximum Rouge2 F1-score (Full Precision)', 
             xlabel = 'Max. F1 score', ylabel = 'Frequency')

    gen_hist(fname = 'hist_max_fscore_quant.png', data = max_fscores_q,
             title = 'Maximum Rouge2 F1-score (Quantized)', 
             xlabel = 'Max. F1 score', ylabel = 'Frequency')

    gen_hist(fname = 'hist_fscore_comparison.png', data = fscores,
             title = 'Rouge2 F1 score - Quantized vs. Full Precision', 
             xlabel = 'F1 score', ylabel = 'Frequency')

    # ================= 3) ============================