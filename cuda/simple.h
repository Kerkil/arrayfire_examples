#include <cuda.h>

__global__
static void simple_kernel(float *d_y,
                          const float *d_x,
                          const int num)
{
    const int id = blockIdx.x * blockDim.x + threadIdx.x;

    if (id < num) {
        float x = d_x[id];
        float sin_x = sin(x);
        float cos_x = cos(x);
        d_y[id] = (sin_x * sin_x) + (cos_x * cos_x);
    }
}

void inline launch_simple_kernel(float *d_y,
                                 const float *d_x,
                                 const int num)
{
    // Set launch configuration
    const int threads = 256;
    const int blocks = (num / threads) + ((num % threads) ? 1 : 0);
    simple_kernel<<<blocks, threads>>>(d_y, d_x, num);

    // Synchronize and check for error
    cudaError_t err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("CUDA Error(%d): %s\n", err, cudaGetErrorString(err));
        throw (int)err;
    }
}
