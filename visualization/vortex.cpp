#include <stdio.h>
#include <string.h>
#include <arrayfire.h>
#include "../common/progress.h"


using namespace af;

static void vortex(bool console)
{
    // seconds to run demo
    double time_total = 10;

    // initialize parameters
    unsigned particles = 10000;
    float revolutions = 5.0;
    float xAx = 0.7f;
    float yAx = 0.3f;
    float zD = -0.0025f;

    float zero = 0;

    array Z = randu(particles);
    array T = randu(particles) * 2 * af::Pi;

    array X = xAx * Z * cos(T) + 0.5;
    array Y = yAx * Z * sin(T) + 0.5;

    unsigned iter = 0;
    timer t = timer::start();

    // While time has not elapsed
    while (progress(iter, t, time_total)) {
        Z = Z + zD;

        // Remove old points and add new points
        Z = array(Z < zero) + Z * array(Z >= zero);

        // Update the X and Y points
        array TEMP = T + 2 * af::Pi * revolutions * .00025 * (iter++);
        X = xAx * Z * cos(TEMP) + 0.5;
        Y = yAx * Z * sin(TEMP) + 0.5;
        if (!console) plot3(X, Z, Y);
        else           eval(X, Y, Z);
    }
}



int main(int argc, char* argv[])
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {
        af::deviceset(device);
        af::info();
        printf("simulation of a vortex of particles\n");
        vortex(console);
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
