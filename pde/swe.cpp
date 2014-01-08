#include <math.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <arrayfire.h>
#include "../common/progress.h"

using namespace af;

static void swe(bool console)
{
    double time_total = 20; // run for N seconds

    // Grid length, number and spacing
    const unsigned Lx = 512, nx = Lx + 1;
    const unsigned Ly = 512, ny = Ly + 1;

    const float dx = Lx / (nx - 1);
    const float dy = Ly / (ny - 1);

    array ZERO = constant(0,nx, ny);
    array um = ZERO, vm = ZERO;

    unsigned io = (unsigned)floor(Lx / 5.0f),
             jo = (unsigned)floor(Ly / 5.0f),
             k = 20;

    array x = tile(moddims(seq(nx),nx,1), 1,ny);
    array y = tile(moddims(seq(ny),1,ny), nx,1);
    array etam = 0.01f * exp((-((x - io) * (x - io) + (y - jo) * (y - jo))) / (k * k));
    array eta = etam;

    float dt = 0.5;

    // conv kernels
    unsigned diff_x_dims[] = {3, 1}, diff_y_dims[] = {1, 3}, lap_dims[] = {3, 3};
    float h_diff_kernel[] = {9.81f * (dt / dx), 0, -9.81f * (dt / dx)};
    float h_lap_kernel[] = {0, 1, 0, 1, -4, 1, 0, 1, 0};

    timer t = timer::start();
    unsigned iter = 0;

    while (progress(iter, t, time_total)) {

        // compute
        array up = um + convolve(eta, 2, diff_x_dims, h_diff_kernel);
        array vp = um + convolve(eta, 2, diff_y_dims, h_diff_kernel);

        array e = convolve(eta, 2, lap_dims, h_lap_kernel);
        array etap = 2 * eta - etam + (2 * dt * dt) / (dx * dy) * e;
        etam = eta;
        eta = etap;

        if (!console) {
            // viz
            fig("color","mood");
            fig("sub",2,1,1);   image(eta);          fig("title","wave propogation");
            fig("sub",2,1,2);   plot3(eta, up, vp);  fig("title","gradients versus magnitude");

        } else eval(eta, up, vp);

        iter++;
    }
}

int main(int argc, char* argv[])
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {
        af::deviceset(device);
        af::info();

        printf("simulation of shallow water equations\n");
        swe(console);

    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    return 0;
}
