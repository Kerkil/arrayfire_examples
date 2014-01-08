#include <stdio.h>
#include <arrayfire.h>
#include <af/utils.h>

using namespace af;

int main(int argc, char ** argv)
{
    try {
        int device = argc > 1 ? atoi(argv[1]) : 0;
        af::deviceset(device);
        af::info();

        printf("\n=== ArrayFire signed(s32) / unsigned(u32) Integer Example ===\n");

        int h_A[] = {1, 2, 4, -1, 2, 0, 4, 2, 3};
        int h_B[] = {2, 3, -5, 6, 0, 10, -12, 0, 1};
        array A = array(3, 3, h_A);
        array B = array(3, 3, h_B);

        printf("--\nSub-refencing and Sub-assignment\n");
        print(A);
        print(A.col(0));
        print(A.row(0));
        A(0) = 11;
        A(1) = 100;
        print(A);
        print(B);
        A(1,span) = B(2,span);
        print(A);

        printf("--Bit-wise operations\n");
        // Returns an array of type s32
        print(A & B);
        print(A | B);
        print(A ^ B);

        printf("\n--Logical operations\n");
        // Returns an array of type b8
        print(A && B);
        print(A || B);

        printf("\n--Transpose\n");
        print(A);
        print(A.T());

        printf("\n--Flip Vertically / Horizontally\n");
        print(A);
        print(flip(A,0));
        print(flip(A,1));

        printf("\n--Sum along columns\n");
        print(A);
        print(sum(A));

        printf("\n--Product along columns\n");
        print(A);
        print(mul(A));

        printf("\n--Minimum along columns\n");
        print(A);
        print(min(A));

        printf("\n--Maximum along columns\n");
        print(A);
        print(max(A));

        printf("\n--Minimum along columns with index\n");
        print(A);

        array out, idx;
        min(out, idx, A);
        print(out);
        print(idx);

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
