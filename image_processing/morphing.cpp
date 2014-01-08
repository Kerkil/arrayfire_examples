#include <stdio.h>
#include <arrayfire.h>
#include <af/utils.h>

using namespace af;

array morphopen(const array& img, const array& mask)
{
    return dilate(erode(img, mask), mask);
}

array morphclose(const array& img, const array& mask)
{
    return erode(dilate(img, mask), mask);
}

array morphgrad(const array& img, const array& mask)
{
    return (dilate(img, mask) - erode(img, mask));
}

array tophat(const array& img, const array& mask)
{
    return (img - morphopen(img, mask));
}

array bottomhat(const array& img, const array& mask)
{
    return (morphclose(img, mask) - img);
}

array border(const array& img, const unsigned left, const unsigned right,
                               const unsigned top, const unsigned bottom,
                               const float value = 0.0)
{
    if(img.dims(0) < (int)(top + bottom))
        std::cerr << "input does not have enough rows" << std::endl;
    if(img.dims(1) < (int)(left + right))
        std::cerr << "input does not have enough columns" << std::endl;

    array ret = img.copy();
    ret(seq(top), span, span, span)              = value;
    ret(af::end - seq(bottom), span, span, span) = value;
    ret(span, seq(left), span, span)             = value;
    ret(span, af::end - seq(right), span, span)  = value;

    return ret;
}

array border(const array& img, const unsigned w, const unsigned h,
                               const float value = 0.0)
{
    return border(img, w, w, h, h, value);
}

array border(const array& img, const unsigned size, const float value = 0.0)
{
    return border(img, size, size, size, size, value);
}

array blur(const array& img, const array mask = gaussiankernel(3,3))
{
    array blurred = img.copy();
    blurred = filter(blurred, mask);
    return blurred;
}

// Demonstrates various image morphing manipulations.
static void morphing_demo(bool console)
{

    // load images
    array img_rgb = loadimage("../common/images/atlantis.png", true) / 255.f; // 3 channel RGB       [0-1]

    array mask = constant(1, 5, 5);

    array er = erode(img_rgb, mask);
    array di = dilate(img_rgb, mask);
    array op = morphopen(img_rgb, mask);
    array cl = morphclose(img_rgb, mask);
    array gr = morphgrad(img_rgb, mask);
    array th = tophat(img_rgb, mask);
    array bh = bottomhat(img_rgb, mask);
    array bo = border(img_rgb, 20);
    array bp = border(img_rgb, 20, 30, 40, 50, 0.5);
    array bl = blur(img_rgb, gaussiankernel(5,5));

    if (!console) {
        // colormap, grayscale
        fig("color","default");

        // image operations
        fig("sub",4,3,1);  image(img_rgb);              fig("title","image");
        fig("sub",4,3,2);  image(er);                   fig("title","erode");
        fig("sub",4,3,3);  image(di);                   fig("title","dilate");
        fig("sub",4,3,4);  image(op);                   fig("title","open");
        fig("sub",4,3,5);  image(cl);                   fig("title","close");
        fig("sub",4,3,6);  image(gr);                   fig("title","grad");
        fig("sub",4,3,7);  image(bp);                   fig("title","indi border");
        fig("sub",4,3,8);  image(th);                   fig("title","tophat");
        fig("sub",4,3,9);  image(bh);                   fig("title","blackhat");
        fig("sub",4,3,10); image(bl);                   fig("title","blur");

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
        printf("** ArrayFire Image Morphing Demo **\n\n");
        morphing_demo(console);

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
