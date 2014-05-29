#include <stdio.h>
#include <arrayfire.h>
#include <af/utils.h>

using namespace af;

static void fast_demo(bool console)
{
    // Load a color image
    array img_color = loadimage("../common/images/lena512x512.jpg", true);
    // Convert the image from RGB to gray-scale
    array img_gray = colorspace(img_color, af_gray, af_rgb);
    // For visualization in ArrayFire, color images must be in the [0.0f-1.0f] interval
    img_color /= 255.f;

    // Calculate image gradients
    array ix, iy;
    grad(ix, iy, img_gray);

    // Compute second-order derivatives
    array ixx = ix * ix;
    array ixy = ix * iy;
    array iyy = iy * iy;

    // Compute a Gaussian kernel with standard deviation of 1.0 and length of 5 pixels
    // These values can be changed to use a smaller or larger window
    array gauss_filt = gaussiankernel(5, 5, 1.0, 1.0);

    // Filter second-order derivatives with Gaussian kernel computed previously
    ixx = convolve(ixx, gauss_filt);
    ixy = convolve(ixy, gauss_filt);
    iyy = convolve(iyy, gauss_filt);

    // Calculate trace
    array tr = ixx + iyy;
    // Calculate determinant
    array det = ixx * iyy - ixy * ixy;

    // Calculate Harris response
    array response = det - 0.04f * (tr * tr);

    // Gets maximum response for each 3x3 neighborhood
    array max_resp = maxfilt(response, 3, 3);

    // Discard responses that are not greater than threshold
    array corners = response > 1e5f;
    corners = corners * response;

    // Discard responses that are not equal to maximum neighborhood response,
    // scale them to original response value
    corners = (corners == max_resp) * corners;

    // Gets host pointer to response data
    float* h_corners = corners.host<float>();

    unsigned good_corners = 0;

    // Draw draw_len x draw_len crosshairs where the corners are
    const int draw_len = 3;
    for (int y = draw_len; y < img_color.dims(0) - draw_len; y++) {
        for (int x = draw_len; x < img_color.dims(1) - draw_len; x++) {
            // Only draws crosshair if is a corner
            if (h_corners[x * corners.dims(0) + y] > 1e5f) {
                // Draw horizontal line of (draw_len * 2 + 1) pixels centered on the corner
                // Set only the first channel to 1 (green lines)
                img_color(y, seq(x-draw_len, x+draw_len), 0) = 0.f;
                img_color(y, seq(x-draw_len, x+draw_len), 1) = 1.f;
                img_color(y, seq(x-draw_len, x+draw_len), 2) = 0.f;

                // Draw vertical line of (draw_len * 2 + 1) pixels centered on  the corner
                // Set only the first channel to 1 (green lines)
                img_color(seq(y-draw_len, y+draw_len), x, 0) = 0.f;
                img_color(seq(y-draw_len, y+draw_len), x, 1) = 1.f;
                img_color(seq(y-draw_len, y+draw_len), x, 2) = 0.f;

                good_corners++;
            }
        }
    }

    printf("Corners found: %u\n", good_corners);

    if (!console) {
        // Previews color image with green crosshairs
        image(img_color);
    }
}

int main(int argc, char** argv)
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {
        af::deviceset(device);
        af::info();
        printf("** ArrayFire FAST Feature Detector Demo **\n\n");
        fast_demo(console);

    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    if (!console) {
        printf("hit [enter]...");
        getchar();
    }
    return 0;
}
