#include <stdio.h>
#include <arrayfire.h>
#include "simple.h"

const int num = 1024;

int main()
{
    // Generate input data
    af::array x = af::randu(num, 1);

    // Get device pointers
    float *d_x = x.device<float>();

    // Allocate data needed for output
    float *d_y = af::array::alloc<float>(num);

    // Finish the tasks arrayfire was doing
    af::sync();

    // Launch kernel to do the following operations
    // y = sin(x)^2 + con(x)^2
    launch_simple_kernel(d_y, d_x, num);

    // Create arrays from output data
    af::array y(num, d_y);

    // Check for errors
    // sin(x)^ + cos(x)^2 == 1
    // The following should print 0
    float err = af::sum<float>(y - 1);
    printf("Error: %f\n", err);
    return 0;
}
