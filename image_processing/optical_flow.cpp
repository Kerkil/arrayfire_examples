#include <string.h>
#include <stdio.h>
#include <arrayfire.h>

using namespace af;

static void diffs(array& Ix, array& Iy, array& It, array I1, array I2)
{
    //  3x3 derivative kernels
    float dx_kernel[] = { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f,
                          0.0f / 6.0f,  0.0f / 6.0f,  0.0f / 6.0f,
                          1.0f / 6.0f,  1.0f / 6.0f,  1.0f / 6.0f
                        };
    float dy_kernel[] = { -1.0f / 6.0f,  0.0f / 6.0f,  1.0f / 6.0f,
                          -1.0f / 6.0f,  0.0f / 6.0f,  1.0f / 6.0f,
                          -1.0f / 6.0f,  0.0f / 6.0f,  1.0f / 6.0f
                        };
    array dx = array(dim4(3, 3), dx_kernel);
    array dy = array(dim4(3, 3), dy_kernel);
    array dt = constant(1,1, 2) / 4.0;

    Ix = convolve(I1, dx) + convolve(I2, dx);
    Iy = convolve(I1, dy) + convolve(I2, dy);
    It = convolve(I2, dt) - convolve(I1, dt);
}

static void optical_flow_demo(bool console)
{
    double time_total = 10; // run for N seconds

    const float h_mean_kernel[] = {1.0f / 12.0f, 2.0f / 12.0f, 1.0f / 12.0f,
                                   2.0f / 12.0f,        0.0f,  2.0f / 12.0f,
                                   1.0f / 12.0f, 2.0f / 12.0f, 1.1f / 12.0f
                                  };
    array mean_kernel = array(dim4(3, 3), h_mean_kernel, afHost);

    array I1 = loadimage("../common/images/circle_left.ppm"); // grayscale
    array I2 = loadimage("../common/images/circle_center.ppm");

    array u = constant(0,I1.dims()), v = constant(0,I1.dims());
    array Ix, Iy, It; diffs(Ix, Iy, It, I1, I2);

    timer time_start, time_last;
    time_start = time_last = timer::start();
    int iter = 0, iter_last = 0;
    double max_rate = 0;

    while (true) {
        iter++;
        array u_ = convolve(u, mean_kernel);
        array v_ = convolve(v, mean_kernel);

        const float alphasq = 0.1f;
        array num = Ix * u_ + Iy * v_ + It;
        array den = alphasq + Ix * Ix + Iy * Iy;

        array tmp = 0.01 * num;
        u = u_ - (Ix * tmp) / den;
        v = v_ - (Iy * tmp) / den;

        if (!console) {
            fig("color","colors");
            fig("sub",2,2,1); image(I1);
            fig("sub",2,2,3); image(I2);
            fig("sub",2,2,2); image(u);
            fig("sub",2,2,4); image(v);
        }

        double elapsed = timer::stop(time_last);
        if (elapsed > 1) {
            double rate = (iter - iter_last) / elapsed;
            double total_elapsed = timer::stop(time_start);
            time_last = timer::start();
            iter_last = iter;
            max_rate = std::max(max_rate, rate);
            if (total_elapsed >= time_total) {
                break;
            }
            if (!console)
                printf("  iterations per second: %.0f   (progress %.0f%%)\n",
                       rate, 100.0f * total_elapsed / time_total);
        }
    }

    if (console) {
        printf(" ### optical_flow %f iterations per second (max)\n", max_rate);
    }

}

int main(int argc, char* argv[])
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {
        af::deviceset(device);
        af::info();
        printf("Horn-Schunck optical flow\n");

        optical_flow_demo(console);
    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

#ifdef WIN32 // pause in Windows
    if (!console) {
        printf("hit [enter]...");
        fflush(stdout);
        getchar();
    }
#endif
    return 0;
}
