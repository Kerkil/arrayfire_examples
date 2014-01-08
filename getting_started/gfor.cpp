// matrix multiply with/without gfor
#include <stdio.h>
#include <arrayfire.h>
#include <af/utils.h>

using namespace af;

static array A, B, C;

static void for_loop()
{
    for (int i = 0; i < A.dims(2); ++i)
        C(span,span,i) = matmul(A(span,span,i), B);
    C.eval();
}
static void gfor_loop()
{
    gfor (array i, A.dims(2))
        C(span,span,i) = matmul(A(span,span,i), B);
    C.eval();
}

int main(int argc, char **argv)
{
    try {
        int device = argc > 1 ? atoi(argv[1]) : 0;
        af::deviceset(device);
        af::info();

        int n = 4;
        int m = 8;
        A = constant(1,n,n,m);
        B = constant(1,n,n);
        C = constant(0,n,n,m);

        printf("time matrix multiply...\n");

        double for_loop_time = timeit(for_loop);
        if (C(0).scalar<float>() != n) throw "value error";
        printf("  for-loop   %.6g seconds\n", for_loop_time);

        double gfor_loop_time = timeit(gfor_loop);
        if (C(0).scalar<float>() != n) throw "value error";
        printf("  gfor-loop  %.6g seconds\n", gfor_loop_time);

        printf("    speedup  %.2gx\n\n", for_loop_time / gfor_loop_time);

        printf("sum...\n");
        int k = 6;
        array D = randu(k,k);
        array E = constant(0,1,k);
        gfor (array i, E.elements())
            E(i) = sum(D(span,i));
        print(D);
        print(E);
        // equivalent (in this case): print(sum(D,0));

        printf("fft...\n");
        array F = randu(n,n,k);
        array G = constant(0,n,n,k, c32);
        gfor (array i, k) {
            G(span,span,i) = fft(F(span, span, i));
        }
        print(G);

    } catch (exception &e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    } catch (const char *s) {
        fprintf(stderr, "%s\n", s);
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
