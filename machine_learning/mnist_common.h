#include <sstream>
#include <algorithm>
#include <utility>
#include "../common/idxio.h"

bool compare(const std::pair<float, int> l,
             const std::pair<float, int> r)
{
    return l.first >= r.first;
}

typedef std::pair<float, int> sort_type;

template<bool expand_labels>
std::string classify(af::array arr, int k)
{
    std::stringstream ss;
    if (expand_labels) {
        af::array vec = arr(af::span, k);
        float *h_vec = vec.host<float>();
        std::vector<sort_type> data;

        for (int i = 0; i < vec.elements(); i++)
            data.push_back(std::make_pair(h_vec[i], i));

        std::stable_sort(data.begin(), data.end(), compare);

        ss << data[0].second;
    } else {
        ss << (int)(arr(k).scalar<float>());
    }
    return ss.str();
}

template<bool expand_labels>
static void setup_mnist(int *num_classes, int *num_train, int *num_test,
                        af::array &train_images, af::array &test_images,
                        af::array &train_labels, af::array &test_labels, float frac)
{
    std::vector<unsigned> idims;
    std::vector<float   > idata;
    read_idx(idims, idata, "../common/data/mnist/images-subset");

    std::vector<unsigned> ldims;
    std::vector<unsigned> ldata;
    read_idx(ldims, ldata, "../common/data/mnist/labels-subset");

    std::reverse(idims.begin(), idims.end());
    unsigned numdims = idims.size();
    af::array images = af::array(af::dim4(numdims, &idims[0]), &idata[0]);
    af::array labels = af::array(ldims[0], &ldata[0]);

    af::array R = af::randu(10000, 1);
    af::array cond = R < std::min(frac, 0.8f);
    af::array train_indices = where( cond);
    af::array test_indices  = where(!cond);

    train_images = images(af::span, af::span, train_indices) / 255;
    test_images  = images(af::span, af::span, test_indices ) / 255;

    *num_classes = 10;
    *num_train = train_images.dims(2);
    *num_test  = test_images.dims(2);

    if (expand_labels) {
        train_labels = af::constant(0, *num_classes, *num_train);
        test_labels  = af::constant(0, *num_classes, *num_test );

        for (int ii = 0; ii < *num_train; ii++) {
            train_labels(labels(train_indices(ii)), ii) = 1;
        }

        for (int ii = 0; ii < *num_test; ii++) {
            test_labels(labels(test_indices(ii)), ii) = 1;
        }
    } else {
        train_labels = labels(train_indices);
        test_labels  = labels( test_indices);
    }

    return;
}

static af::array randidx(int num, int total)
{
    af::array locs;
    do {
        locs = af::where(af::randu(total, 1) < float(num * 2) / total);
    } while (locs.elements() < num);

    return locs(af::seq(num));
}

template<bool expand_labels>
static void display_results(af::array &test_images,
                            af::array &test_output,
                            int num_display)
{
    af::array locs = randidx(num_display, test_images.dims(2));

    af::array disp_in  = test_images(af::span, af::span, locs);
    af::array disp_out = expand_labels ? test_output(af::span, locs) : test_output(locs);

    for (int i = 0; i < 5; i++) {

        int imgs_per_iter = num_display / 5;
        for (int j = 0; j < imgs_per_iter; j++) {

            int k = i * imgs_per_iter + j;
            af::fig("sub", 2, imgs_per_iter / 2, j+1);

            af::image(disp_in(af::span, af::span, k).T());
            std::string pred_name = std::string("Predicted: ");
            pred_name = pred_name + classify<expand_labels>(disp_out, k);
            af::fig("title", pred_name.c_str());
        }

        printf("Press any key to see next set");
        getchar();
    }
}
