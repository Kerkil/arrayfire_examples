#include <stdio.h>
#include <arrayfire.h>
#include <af/utils.h>

using namespace af;

static const float pyramid_kernel[] = {
    1,  4,  6,  4, 1,
    4, 16, 24, 16, 4,
    6, 24, 36, 24, 6,
    4, 16, 24, 16, 4,
    1,  4,  6,  4, 1
};

array pyramid(const array& img, const int level, const bool sampling)
{
    array pyr = img.copy();
    array kernel(5, 5, pyramid_kernel);
    kernel = kernel / 256.f;
    if(sampling) {                              //Downsample
        for(int i = 0; i < level; i++) {
            for(int j = 0; j < pyr.dims(2); j++)
                pyr(span, span, j) = convolve(pyr(span, span, j), kernel);
            pyr = pyr(seq(0, 2, af::end), seq(0, 2, af::end), span);
        }
    } else {                                    // Up sample
        for(int i = 0; i < level; i++) {
            array tmp = constant(0, pyr.dims(0) * 2, pyr.dims(1) * 2, pyr.dims(2));
            tmp(seq(0, 2, af::end), seq(0, 2, af::end), span) = pyr;
            for(int j = 0; j < pyr.dims(2); j++)
                tmp(span, span, j) = convolve(tmp(span, span, j), kernel * 4.f);
            pyr = tmp;
        }
    }
    return pyr;
}

void pyramids_demo(bool console)
{
    array img_rgb = loadimage("../common/images/atlantis.png", true) / 255.f; // 3 channel RGB       [0-1]
    array img_gray = colorspace(img_rgb, af_gray, af_rgb);

    array downc1 = pyramid(img_rgb,  1, true);
    array downc2 = pyramid(img_rgb,  2, true);
    array upc1   = pyramid(img_rgb,  1, false);
    array upc2   = pyramid(img_rgb,  2, false);

    array downg1 = pyramid(img_gray, 1, true);
    array downg2 = pyramid(img_gray, 2, true);
    array upg1   = pyramid(img_gray, 1, false);
    array upg2   = pyramid(img_gray, 2, false);

    if (!console) {
        // colormap, grayscale
        fig("color","default");

        printf("Showing color images...\n");
        // image operations
        fig("sub",3,2,1);   image(img_rgb);               fig("title","color image");
        fig("sub",3,2,2);   image(downc1);                fig("title","downsample 1 level");
        fig("sub",3,2,3);   image(downc2);                fig("title","downsample 2 levels");
        fig("sub",3,2,4);   image(upc1);                  fig("title","upsample 1 level");
        fig("sub",3,2,5);   image(upc2);                  fig("title","upsample 2 level");

        // Force screen refresh (optional)
        fig("draw");

        printf("Press any key for grayscale images\n");
        getchar();

        fig("sub",3,2,1);   image(img_gray);              fig("title","grayscale image");
        fig("sub",3,2,2);   image(downg1);                fig("title","downsample 1 level");
        fig("sub",3,2,3);   image(downg2);                fig("title","downsample 2 levels");
        fig("sub",3,2,4);   image(upg1);                  fig("title","upsample 1 level");
        fig("sub",3,2,5);   image(upg2);                  fig("title","upsample 2 level");

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
        printf("** ArrayFire Image Pyramids Demo **\n\n");
        pyramids_demo(console);

    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    if (!console) {
        printf("Press any key to exit\n");
        getchar();
    }
    return 0;
}
