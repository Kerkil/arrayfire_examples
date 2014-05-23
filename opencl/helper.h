#include <iostream>
#include <fstream>
#include <CL/cl.h>

static std::string get_kernel_string(const char *file_name)
{
    std::ifstream file(file_name);
    std::string kernel;
    std::string line;

    while(std::getline(file, line)) {
        kernel = kernel + line;
        kernel = kernel + "\n";
    }

    return kernel;
}

static cl_context get_context(cl_mem x)
{
    cl_context context;
    cl_int err = clGetMemObjectInfo(x, CL_MEM_CONTEXT, sizeof(cl_context), &context, NULL);
    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to get context from Memory object\n");
        throw (err);
    }
    return context;
}

static cl_program build_program(cl_context context, std::string kernel_string)
{
    cl_int err;
    const char *source = kernel_string.c_str();
    size_t length = kernel_string.size();

    cl_program program = clCreateProgramWithSource(context, 1, &source, &length, &err);
    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to create program\n");
        throw (err);
    }

    err = clBuildProgram(program, 0, NULL, "", NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to build program\n");
        throw (err);
    }

    return program;
}

static cl_kernel create_kernel(cl_program program, const char *kernel_name)
{
    cl_int err;
    cl_kernel kernel = clCreateKernel(program, kernel_name, &err);
    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to create kernel %s\n", err, kernel_name);
        throw (err);
    }
    return kernel;
}

static cl_command_queue create_queue(cl_context context)
{
    cl_device_id device;
    cl_int err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &device, NULL);
    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to get device from context\n");
        throw (err);
    }

    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) {
        printf("OpenCL Error(%d): Failed to command queue\n");
        throw (err);
    }

    return queue;
}
