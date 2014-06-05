#include <sstream>
#include <algorithm>
#include <utility>
#include "../common/idxio.h"

using namespace af;
using namespace std;

bool compare(const std::pair<float, int> l,
             const std::pair<float, int> r)
{
    return l.first >= r.first;
}

typedef std::pair<float, int> sort_type;

std::string classify(array vec)
{
    float *h_vec = vec.host<float>();
    std::vector<sort_type> data;

    for (int i = 0; i < vec.elements(); i++)
        data.push_back(std::make_pair(h_vec[i], i));

    std::stable_sort(data.begin(), data.end(), compare);

    stringstream ss;
    ss << data[0].second;
    return ss.str();
}

static void setup_mnist(int *num_classes, int *num_train, int *num_test,
                        af::array &train_images, af::array &test_images,
                        af::array &train_labels, af::array &test_labels)
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

    af::array R = randu(10000, 1);
    af::array cond = R < 0.6;
    af::array train_indices = where( cond);
    af::array test_indices  = where(!cond);

    train_images = images(span, span, train_indices); // Normalize
    test_images  = images(span, span, test_indices );

    *num_classes = 10;
    *num_train = train_images.dims(2);
    *num_test  = test_images.dims(2);

    train_labels = af::constant(0, *num_classes, *num_train);
    test_labels  = af::constant(0, *num_classes, *num_test );

    for (int ii = 0; ii < *num_train; ii++) {
        train_labels(labels(train_indices(ii)), ii) = 1;
    }

    for (int ii = 0; ii < *num_test; ii++) {
        test_labels(labels(test_indices(ii)), ii) = 1;
    }

    return;
}

static af::array randidx(int num, int total)
{
    af::array locs;
    do {
        locs = where(randu(total, 1) < float(num * 2) / total);
    } while (locs.elements() < num);

    return locs(seq(num));
}

static void display_results(array &test_input,
                            array &test_output,
                            int num_display)
{
    array locs = randidx(num_display, test_output.dims(1));

    array disp_in  = test_input(span, span, locs);
    array disp_out = test_output(span, locs);

    for (int i = 0; i < 5; i++) {

        int imgs_per_iter = num_display / 5;
        for (int j = 0; j < imgs_per_iter; j++) {

            int k = i * imgs_per_iter + j;
            fig("sub", 2, imgs_per_iter / 2, j+1);

            image(disp_in(span, span, k).T());
            string pred_name = string("Predicted: ") + classify(disp_out(span, k));
            fig("title", pred_name.c_str());
        }

        printf("Press any key to see next set");
        getchar();
    }
}
