#include "helper.h"

void inline launch_simple_kernel(float *d_y,
                                 const float *d_x,
                                 const int num)
{


    // Read the OpenCL kernel as a string
    std::string simple_kernel = get_kernel_string("simple.cl");

    // Get OpenCL context from memory buffer and create a Queue
    cl_context context = get_context((cl_mem)d_x);
    cl_command_queue queue = create_queue(context);

    // Build the OpenCL program and get the kernel
    cl_program program = build_program(context, simple_kernel);
    cl_kernel   kernel = create_kernel(program, "simple_kernel");

    cl_int err = CL_SUCCESS;
    int arg = 0;

    // Set input parameters for the kernel
    err |= clSetKernelArg(kernel, arg++, sizeof(cl_mem), &d_y);
    err |= clSetKernelArg(kernel, arg++, sizeof(cl_mem), &d_x);
    err |= clSetKernelArg(kernel, arg++, sizeof(int   ), &num);

    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to set kernel arguments\n", err);
        throw (err);
    }

    // Set launch configuration parameters and launch kernel
    size_t local  = 256;
    size_t global = local * (num / local + ((num % local) ? 1 : 0));
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to enqueue kernel\n", err);
        throw (err);
    }

    err = clFinish(queue);
    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Kernel failed to finish\n", err);
        throw (err);
    }

    return;
}
