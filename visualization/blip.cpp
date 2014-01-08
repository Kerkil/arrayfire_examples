#include <string.h>
#include <stdio.h>
#include <math.h>
#include <arrayfire.h>
#include "../common/progress.h"

using namespace af;

static array blip(array& X, array& Y, float sigma, float phi)
{
    array XX = X * X, YY = Y*Y;
    float sigma2 = sigma * sigma, sigma4 = sigma2 * sigma2;
    return -((XX + YY - 2 * sigma2) / sigma4) *
        exp( (-XX + -YY) / (2 * sigma2) ) +
        cos(sqrt(XX + YY) * 3 + phi) / 10;
}

int main(int argc, char *argv[])
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {
        af::deviceset(device);
        af::info();

        double time_total = 10;

        // blip parameters
        float mn = -10, mx = 10;
        int N = 100;
        seq v(mn, (mx - mn) / N, mx);
        array X = tile(moddims(v,N,1),1,N);
        array Y = tile(moddims(v,1,N),N,1);

        // timers for calculating rate
        timer t = timer::start();
        unsigned iter = 0;
        float ts = 0;

        // While time has not elapsed
        while (progress(iter, t, time_total)) {
            ts += 0.01f;
            iter++;
            float phi = ts, sigma = 2.0f + cos(ts);
            array B = blip(X, Y, sigma, phi);

            if (console) B.eval();
            else {
                // graphical mode
                fig("color","heat");
                fig("sub",2,1,1); surface(B);
                fig("sub",2,3,2); plot2(B(span, 0));
                fig("sub",2,3,4); plot2(B(span, N / 2));
                fig("sub",2,3,6); image(B);
            }
        }

    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    return 0;
}
