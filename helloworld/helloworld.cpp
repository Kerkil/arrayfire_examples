#include <stdio.h>
#include <arrayfire.h>
#include <af/utils.h>

using namespace af;

int main(int argc, char **argv)
{
    try {
        int device = argc > 1 ? atoi(argv[1]) : 0;
        af::deviceset(device);
        af::info();

        printf("\ncreate a 5-by-3 matrix of random floats on the GPU\n");
        array A = randu(5,3);
        print(A);

        printf("element-wise arithmetic\n");
        array B = sin(A) + 0.1;
        print(B);

        printf("Fourier transform the result\n");
        array C = fft(B);
        print(C);

        printf("grab last row\n");
        array c = C.row(end);
        print(c);

        printf("zero out every other column\n");
        B(span, seq(0,2,end)) = 0;
        printf("negate the first three elements of middle column\n");
        B(seq(3), 1) = B(seq(3), 1) * -1;
        print(B);

        printf("create 2-by-3 matrix from host data\n");
        float d[] = { 1, 2, 3, 4, 5, 6 };
        array D(2, 3, d, afHost);
        print(D);

        printf("copy last column onto first\n");
        D.col(0) = D.col(end);
        print(D);

        // pull back to CPU
        float* d_ = D.host<float>();

        // free up the memory allocated when copying to the host.
        array::free(d_);

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
