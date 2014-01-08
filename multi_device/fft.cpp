#include <stdio.h>
#include <arrayfire.h>
#include <math.h>
using namespace af;

#ifndef AFCL
static int n = 2048; // n-by-n matrix for benchmarking
static int ndevice; // how many devices to use
static void bench()
{
    for (int i = 0; i < ndevice; ++i) {
        deviceset(i);              // switch to device
        array x = randu(n,n,c32);  // generate random complex input
        array y = fft(x);          // analyze signal of each column
    }
}
#endif

int main(int argc, char **argv)
{
    try {
        info();
#ifndef AFCL
        int total_device = devicecount();
        for (ndevice = 1; ndevice <= total_device; ++ndevice) {
            double time_s = timeit(bench); // seconds
            double gflops = ndevice * 5 * n * n * log(double(n)) / log(2.0) / (1e9 * time_s);
            printf("%dx%d random number gen and complex FFT on %d devices...  %.1f gflops\n",
                   n, n, ndevice, gflops);
        }
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
// fft gflops calc: http://www.fftw.org/speed/method.html
