# ECE654_Project
Project: Enabling Deployment of Generative Language Models on Resource-Constrained Devices

To download pre-trained models:
`python download.py --model [8M/3M]`

To quantize the pre-trained models (defaults to 3M model; change hard-coded paths on lines 33, 34, and 36 for different model):
`python quantize_model.py`

To evaluate the quantized model (defaults to 3M model; change hard-coded paths on lines 74, 75, 76 for different model):
`python eval_quantized.py`

Note that MCU code is designed for Texas Instruments' TMS570LC4357 Microcontroller using the CCS 10.0.0.00010 toolchain.
