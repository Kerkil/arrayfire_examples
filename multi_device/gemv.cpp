#include <iostream>
#include <arrayfire.h>
#include <stdio.h>
#include <assert.h>

using namespace af;
#ifndef AFCL
static void multi_Sgemv(int iterations, int ndevice, array *AMatrix, int m, const float* X, int n, float *Y)
{
    array *YVector = new array[ndevice];
    float **YVector_host = new float* [ndevice];

    for (int i = 0 ; i < iterations; i++) {
        // Multiply each piece of A*x
        for (int idx = 0; idx < ndevice; idx++) {
            deviceset(idx);
            YVector[idx] = matmul(AMatrix[idx], array(n/ndevice, X + idx*(n/ndevice), afHost));
        }

        // Copy partial results back to host (implicitly blocks until done)
        for (int idx = 0; idx < ndevice; idx++) {
            deviceset(idx);
            YVector_host[idx] = YVector[idx].host<float>();
        }

        // Accumulate result in Y
        for (int i = 0; i < m; i++) {
            Y[i] = 0;
            for (int j = 0; j < ndevice; j++)
                Y[i] +=  YVector_host[j][i];
        }

        // cleanup
        for (int idx = 0; idx < ndevice; idx++)
            array::free(YVector_host[idx]);
    }
    delete [] YVector;
    delete [] YVector_host;
}

static void ones(float *X, int n)
{
    while (n--)
        *(X++) = 1;
}

#define MB (1024 * 1024)
#define mb(x)   (unsigned)((x) / MB + !!((x) % MB))
#endif

int main(int argc, char  **argv)
{
    try {
        #ifndef AFCL
        printf("Multi-GPU Matrix-Vector Multiply: y = A*x\n\n"
               "The system matrix 'A' is distributed across the available devices.\n"
               "Each iteration pushes 'x' to the devices, multiplies against the matrix 'A',\n"
               "and pulls the result 'y' back to the host.\n\n");
        info();

        int iterations = 1000;

        // Get number of active GPUs in the system
        int ndevice = devicecount();
        if (ndevice == 1) {
            printf("found one device, exiting example\n");
            return 0;
        }

        int n = ndevice*7000;
        printf("size(A)=[%d,%d]  (%u mb)\n", n, n, mb(n * n * sizeof(float)));

        printf("\nBenchmarking........\n\n");
        // assume A,X are all ones (read from disk, memcpy from cpu ram, etc.)
        float *A = new float[n*n], *X = new float[n], *Y = new float[n];
        ones(A, n*n);
        ones(X, n*1);

        // Distribute A partitions across devices
        array *AMatrix = new array[ndevice];
        for (int idx = 0; idx < ndevice; idx++) {
            deviceset(idx);
            AMatrix[idx] = array(n, n/ndevice, A + (n*n/ndevice * idx), afHost);
        }
        delete[] A; // done with A, keep X to push each iteration

        // do matrix-vector multiply using all GPUs
        af::sync();
        timer::start();
        multi_Sgemv(iterations, ndevice, AMatrix, n, X, n, Y);
        af::sync();
        printf("Average time for %d iterations : %g seconds\n", iterations, timer::stop() / iterations);

        // ensure results correct
        for (int i = 0; i < n; ++i)
            assert(Y[i] == n);

        delete[] X; delete[] Y; // cleanup
        #else
        printf("Multi gpu support for ArrayFire OpenCL will be available in final release\n");
        #endif

    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    #ifdef WIN32 // pause in Windows
    if (!(argc == 2 && argv[1][0] == '-')) {
        printf("hit [enter]...");
        fflush(stdout);
        getchar();
    }
    #endif
    return 0;
}
