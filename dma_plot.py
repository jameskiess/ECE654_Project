# Plot the DMA timing results
import matplotlib.pyplot as plt 
import numpy as np


if __name__ == "__main__":
    # DMA results
    dma_req_time = [11, 10, 10, 15, 19, 28, 43, 77]
    dma_tf_time = [252, 505, 1010, 2020, 1993, 3985, 8009, 15969]
    cpu_tf_time = [18, 30, 56, 107, 210, 414, 824, 1644]
    tf_sizes = [1, 2, 4, 8, 16, 32, 64, 128]

    matmul_time = [5886, 16487, 46824, 129008, 369003, 1038790, 2929963, 8266346]

    dma_req_time = np.array(dma_req_time)
    dma_tf_time = np.array(dma_tf_time)
    cpu_tf_time = np.array(cpu_tf_time)
    tf_sizes = np.array(tf_sizes)
    matmul_time = np.array(matmul_time)

    # Generate plot - DMA transfers
    fig = plt.figure(figsize=(12,8), facecolor='white')
    ax = fig.add_subplot(1,1,1, frameon=False)
    ax.set_title('DMA Transfer Times', fontsize=14)
    ax.set_xlabel('Transfer size (KB)', fontsize=12)
    ax.set_ylabel('Latency (microseconds)', fontsize=12)
    plt.plot(tf_sizes, dma_tf_time)
    plt.grid()
    fig.tight_layout()
    plt.savefig('dma_transfer_plot.png')
    plt.close(fig)

    # Generate plot - DMA request vs. CPU transfer time
    fig = plt.figure(figsize=(12,8), facecolor='white')
    ax = fig.add_subplot(1,1,1, frameon=False)
    ax.set_title('CPU Data Transfer Latency vs. DMA Request Latency', fontsize=12)
    ax.set_xlabel('Transfer size (KB)', fontsize=12)
    ax.set_ylabel('Latency (microseconds)', fontsize=12)
    plt.plot(tf_sizes, cpu_tf_time, label='CPU Transfer')
    plt.plot(tf_sizes, dma_req_time, label='DMA Request')
    ax.legend()
    plt.grid()
    fig.tight_layout()
    plt.savefig('cpu_dma_comparison.png')
    plt.close(fig)

    # Generate plot - DMA transfer vs. MatMul execution time
    fig = plt.figure(figsize=(12,8), facecolor='white')
    ax = fig.add_subplot(1,1,1, frameon=False)
    ax.set_title('DMA Transfer Latency vs. Matrix Multiplication Latency', fontsize=12)
    ax.set_xlabel('Size (KB)', fontsize=12)
    ax.set_ylabel('Latency (milliseconds)', fontsize=12)
    ax.set_yscale('log')
    plt.plot(tf_sizes, dma_tf_time / 1000, label='DMA Transfer')
    plt.plot(tf_sizes, matmul_time / 1000, label='Matrix Multiplication')
    ax.legend()
    plt.grid()
    fig.tight_layout()
    plt.savefig('dma_matmul_comparison.png')
    plt.close(fig)