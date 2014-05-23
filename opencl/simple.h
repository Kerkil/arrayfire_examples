#include "helper.h"

void inline launch_simple_kernel(float *d_y,
                                 const float *d_x,
                                 const int num)
{


    std::string simple_kernel = get_kernel_string("simple.cl");

    cl_context context = get_context((cl_mem)d_x);
    cl_context context2 = get_context((cl_mem)d_y);

    cl_program program = build_program(context, simple_kernel);
    cl_kernel   kernel = create_kernel(program, "simple_kernel");
    cl_command_queue queue = create_queue(context);

    cl_int err = CL_SUCCESS;
    int arg = 0;

    err |= clSetKernelArg(kernel, arg++, sizeof(cl_mem), &d_y);
    err |= clSetKernelArg(kernel, arg++, sizeof(cl_mem), &d_x);
    err |= clSetKernelArg(kernel, arg++, sizeof(int   ), &num);

    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to set kernel arguments\n", err);
        throw (err);
    }

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
