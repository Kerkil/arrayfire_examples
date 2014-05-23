__kernel
void simple_kernel(__global float *d_y,
                   __global const float *d_x,
                   const int num)
{
    const int id = get_global_id(0);

    if (id < num) {
        float x = d_x[id];
        float sin_x = sin(x);
        float cos_x = cos(x);
        d_y[id] = (sin_x * sin_x) + (cos_x * cos_x);
    }
}
