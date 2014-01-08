#include <stdio.h>
#include <arrayfire.h>

using namespace af;


// Split a MxNx3 image into 3 separate channel matrices.
static void channel_split(array& rgb, array& outr, array& outg, array& outb) {
    outr = rgb(span, span, 0);
    outg = rgb(span, span, 1);
    outb = rgb(span, span, 2);
}

// 5x5 sigma-3 gaussian blur weights
static const float h_gauss[] = {
    0.0318,  0.0375,  0.0397,  0.0375,  0.0318,
    0.0375,  0.0443,  0.0469,  0.0443,  0.0375,
    0.0397,  0.0469,  0.0495,  0.0469,  0.0397,
    0.0375,  0.0443,  0.0469,  0.0443,  0.0375,
    0.0318,  0.0375,  0.0397,  0.0375,  0.0318,
};

// 3x3 sobel weights
static const float h_sobel[] = {
    -2.0, -1.0,  0.0,
    -1.0,  0.0,  1.0,
     0.0,  1.0,  2.0
};

// Demonstrates various image manipulations.
static void img_test_demo(bool console)
{

    // load convolution kernels
    array gauss_k = array(5, 5, h_gauss);
    array sobel_k = array(3, 3, h_sobel);

    // load images
    array img_gray = loadimage("../common/images/trees_ctm.jpg", false);         // 1 channel grayscale [0-255]
    array img_rgb  = loadimage("../common/images/sunset_emp.jpg", true) / 255.f; // 3 channel RGB       [0-1]

    // rgb channels
    array rr, gg, bb;
    channel_split(img_rgb, rr, gg, bb);

    // hsv channels
    array hsv = colorspace(img_rgb,af_hsv, af_rgb);
    array hh, ss, vv;
    channel_split(hsv, hh, ss, vv);

    // image histogram equalization
    array ihist = histogram(img_gray, 256);
    array inorm = histequal(img_gray, ihist);

    if (!console) {
        // colormap, grayscale
        fig("color","default");

        // image operations
        fig("sub",4,2,1);  image(img_rgb);                          fig("title","color image");
        fig("sub",4,2,2);  image(bb);                               fig("title","blue channel");
        fig("sub",4,2,3);  image(ss);                               fig("title","saturation");

        fig("sub",4,2,4);  image(convolve(img_gray, gauss_k));      fig("title","smooth");
        fig("sub",4,2,5);  image(rotate(img_gray, Pi / 2, false));  fig("title","rotate");
        fig("sub",4,2,6);  image(img_gray < 130);                   fig("title","binary thresholding");

        fig("sub",4,2,7);  image(inorm);                            fig("title","hist eq");
        fig("sub",4,2,8);  image(abs(convolve(img_gray, sobel_k))); fig("title","edge detection");

        // Force screen refresh (optional)
        fig("draw");
    }
}



int main(int argc, char** argv)
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {
        af::deviceset(device);
        af::info();
        printf("** ArrayFire Image Demo **\n\n");
        img_test_demo(console);

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
