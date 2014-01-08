#include <stdio.h>
#include <arrayfire.h>
#include <af/utils.h>

using namespace af;

int main(int argc, char **argv)
{

#ifdef AFCL
        printf("This example doesn't work with ArrayFire OpenCL.\n");
#else
    try {

        int device = argc > 1 ? atoi(argv[1]) : 0;
        af::deviceset(device);
        af::info();

        // This example needs DLA license to work.
        printf("\n======= ArrayFire DLA Examples =====\n\n");

        printf("--ArrayFire LU decomposition\n\n");
        int n = 3, m = 6;
        n = 4;
        array in, out;
        array l, u, p;

        in = randu(m, n);
        out = lu(in); // Packed output
        lu(l, u, p, in); // Unpacked output with pivoting

        print(in);
        print(out);
        print(l);
        print(u);
        print(p);

        printf("--ArrayFire Solve\n\n");
        // Generate random data
        n = 6;
        array A = randu(n, n);
        array B = randu(n, n);

        // Solve: A*X = B
        array X = solve(A, B);

        print(A);
        print(B);
        print(X);

        // Find the maximum of absolute error
        float res = max<float>(abs(matmul(A, X) - B));
        printf("\nMaximum of abolute error: %f\n", res);

    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    // This part requires ArrayFire Pro license.
    try {
        printf("--ArrayFire Eigen value decomposition\n\n");
        int n = 3;
        array in;
        array val, vec;

        in = randu(n, n);

        // Eigen values in a vector
        print(eigen(in));

        // Find the Eigen values and Eigen Vectors
        // Eigen values are along the diagonal of a diagonal matrix
        // The corresponding eigen vectors are in the appropriate column
        eigen(val, vec, in);

        // Print Eigen values and Eigen Vectors
        print(in);
        print(val);
        print(vec);
    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
    }
#endif
    #ifdef WIN32 // pause in Windows
    if (!(argc == 2 && argv[1][0] == '-')) {
        printf("hit [enter]...");
        fflush(stdout);
        getchar();
    }
    #endif

    return 0;
}
