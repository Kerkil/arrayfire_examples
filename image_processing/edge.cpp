#include <stdio.h>
#include <arrayfire.h>
#include <af/utils.h>

using namespace af;

void prewitt(array &mag, array &dir, const array &in)
{
    float h1[] = { 1, 1, 1};
    float h2[] = {-1, 0, 1};

    // Find the gradients
    array Gy = convolve(3, h2, 3, h1, in)/6;
    array Gx = convolve(3, h1, 3, h2, in)/6;

    // Find magnitude and direction
    mag = hypot(Gx, Gy);
    dir = atan2(Gy, Gx);
}

void sobel(array &mag, array &dir, const array &in)
{
    float h1[] = { 1, 2,  1};
    float h2[] = { 1, 0, -1};

    // Find the gradients
    array Gy = convolve(3, h2, 3, h1, in)/8;
    array Gx = convolve(3, h1, 3, h2, in)/8;

    // Find magnitude and direction
    mag = hypot(Gx, Gy);
    dir = atan2(Gy, Gx);
}

// Non maximum suppression
array nonmax_suppress(const array &mag, const array &dir)
{
    array new_mag = mag;

    // Convert radios to degrees and round to multiples of 45
    array phase = (45 * round(4 * (dir / af::Pi)));
    phase = phase + (phase < 0) * 180;
    phase = phase * (phase != 180);

    // Boolean masks for each direction
    array cond0xx = (phase ==   0).as(f32); // E-W
    array cond45x = (phase ==  45).as(f32); // NE-SW
    array cond90x = (phase ==  90).as(f32); // N-S
    array cond135 = (phase == 135).as(f32); // NW-SE

    int nrows = mag.dims(0);
    int ncols = mag.dims(1);

    // Generate grid denoting row and column index of each location
    array rows = tile(array(seq(nrows)), 1, ncols);
    array cols = tile(array(seq(ncols)), 1, nrows).T();

    // Find the index offsets for the neighbors
    array row_locs = cond0xx*(0) + cond45x*(1) + cond90x*(1) + cond135*(1);
    array col_locs = cond0xx*(1) - cond45x*(1) + cond90x*(0) + cond135*(1);

    // Find the index locations of neighbors
    array row_prev = rows - row_locs;
    array col_prev = cols - col_locs;

    array row_next = rows + row_locs;
    array col_next = cols + col_locs;

    // Calculate the linear indices for neighbors
    array indx_curr = (cols) * nrows + rows;
    array indx_prev = (col_prev) * nrows + (row_prev);
    array indx_next = (col_next) * nrows + (row_next);

    // Out of bounds conditions
    array rows_prev_oob = (row_prev < 0) || (row_prev >= nrows);
    array cols_prev_oob = (col_prev < 0) || (col_prev >= ncols);

    array rows_next_oob = (row_next < 0) || (row_next >= nrows);
    array cols_next_oob = (col_next < 0) || (col_next >= ncols);

    array indx_prev_oob = rows_prev_oob || cols_prev_oob;
    array indx_next_oob = rows_next_oob || cols_next_oob;

    // Replace out of bound pixels with current pixels
    indx_prev = indx_prev_oob * indx_curr + !indx_prev_oob * indx_prev;
    indx_next = indx_next_oob * indx_curr + !indx_next_oob * indx_next;

    // Get magnitudes of neighbors
    array mag_prev = mag(indx_prev);
    array mag_next = mag(indx_next);

    // Check if the current pixel has magnitude greater than neighbors
    array prev_cond = round(10 * mag) >= round(10 * mag_prev);
    array next_cond = round(10 * mag) >= round(10 * mag_next);
    new_mag = mag * prev_cond * next_cond;

    return new_mag;
}


array hist_thresh(const array &mag, const array &new_mag,
                  double lim = 0.8, double ratio = 0.4)
{

    int nbins = 25;

    // Generate the cdf of the histogram
    array hist = histogram(mag, nbins);
    array cdf = accum(hist);

    float *h_cdf = cdf.host<float>();
    float high_thresh = 0;

    // iterate through cdf and find the cutoff location
    for (int i = 0; i < nbins; i++) {
        if (h_cdf[i] > lim *  mag.elements()) {

            // Find the pixel value at the cutoff location
            float minval = min<float>(mag);
            float maxval = max<float>(mag);
            float binwidth = (maxval - minval) / nbins;
            high_thresh = minval + binwidth * i;

            break;
        }
    }
    free(h_cdf);

    // Calculate low threshold from high threshold
    float low_thresh = high_thresh * ratio;

    // Find connected components based on low threshold
    array labels = regions(new_mag > low_thresh);

    // Filter out the componenets that have pixels greater than high threshold
    array uniq_labels = setunique(labels * (new_mag > high_thresh));

    // Mark the filtered out components
    array res = new_mag * 0;
    for (int i = 1; i < uniq_labels.elements(); i++) res += (labels == uniq_labels(i));
    return res;
}

array edge(const array &in, int method = 0)
{
    int w = 5;
    if (in.dims(0) <  512) w = 3;
    if (in.dims(0) > 2048) w = 7;

    int h = 5;
    if (in.dims(0) <  512) h = 3;
    if (in.dims(0) > 2048) h = 7;

    array ker = gaussiankernel(w, h);
    array smooth = convolve(in, ker);
    array mag, dir;

    switch(method) {
    case  1: prewitt(mag, dir, smooth); break;
    case  2: sobel(mag, dir, smooth);   break;
    case  3: prewitt(mag, dir, smooth);   break;
    default: throw af::exception("Unsupported type");
    }

    if (method < 3) {
        float thresh = mean<float>(mag);
        return (mag > thresh);
    }

    array new_mag = nonmax_suppress(mag, dir);
    array res = hist_thresh(mag, new_mag, 0.85);
    return res;
}

int main()
{
    try {

        array in = loadimage("../common/images/lena.ppm", false);

        fig("sub", 2, 2, 1); image(in);                   fig("title", "Input");
        fig("sub", 2, 2, 2); image(edge(in, 1));          fig("title", "Prewitt");
        fig("sub", 2, 2, 3); image(edge(in, 2));          fig("title", "Sobel");
        fig("sub", 2, 2, 4); image(edge(in, 3));          fig("title", "Canny");

        printf("Hit enter to finish\n");
        getchar();
    } catch (af::exception &ae) {
        std::cout << ae.what() << std::endl;
    }
    return 0;
}
