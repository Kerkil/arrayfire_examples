#include <arrayfire.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <af/utils.h>
#include <math.h>
#include "mnist_common.h"

using namespace af;

// Get accuracy of the predicted results
float accuracy(const array& predicted, const array& target)
{
    return 100 * count(predicted == target) / target.elements();
}

array naive_bayes(const array &train_feats, const array &test_feats, const array &train_labels)
{
    int num_labels = 10;

    int feat_len = train_feats.dims(0);
    int num_test = test_feats.dims(1);

    // Get mean and variance from trianing data
    array mu  = constant(0, feat_len, num_labels);
    array sig2 = constant(0, feat_len, num_labels);
    for (int ii = 0; ii < num_labels; ii++) {
        array idx = where(train_labels == ii);
        array train_feats_ii = train_feats(span, idx);

        mu(span, ii)  = mean(train_feats_ii, 1);

        // Some pixels are always 0. Add a small variance.
        sig2(span,ii) = var(train_feats_ii, 0, 1) + 0.01;
    }

    // Predict the probabilities for testing data
    // Using log of probabilities to reduce rounding errors
    array log_probs = constant(1, num_test, num_labels);
    for (int ii = 0; ii < num_labels; ii++) {

        // Tile the current mean and variance to the testing data size
        array Mu  = tile(mu (span, ii), 1, num_test);
        array Sig2 = tile(sig2(span, ii), 1, num_test);

        array Df = test_feats - Mu;
        array log_P =  (-(Df * Df) / (2 * Sig2))  - log(sqrt(2 * af::Pi * Sig2));

        // Accumulate the probabilities
        log_probs(span, ii) = sum(log_P).T();
    }

    // Get the location of the maximum value
    array val, idx;
    max(val, idx, log_probs, 1);
    return idx.as(f32);
}

void naive_bayes_demo(bool console)
{
    array train_images, train_labels;
    array test_images, test_labels;
    int num_train, num_test, num_classes;

    // Load mnist data
    setup_mnist<false>(&num_classes, &num_train, &num_test,
                       train_images, test_images,
                       train_labels, test_labels);

    int feature_length = train_images.elements() / num_train;
    array train_feats = moddims(train_images, feature_length, num_train);
    array test_feats  = moddims(test_images , feature_length, num_test );

    // Preprocess
    train_feats = train_feats + tile(min(train_feats, 1), 1, num_train);
    test_feats  =  test_feats + tile(min(test_feats , 1), 1, num_test );

    // Get the predicted results
    array res_labels = naive_bayes(train_feats, test_feats, train_labels);

    // Results
    printf("Accuracy on testing  data: %2.2f\n",
           accuracy(res_labels , test_labels));

    if (!console) {
        display_results<false>(test_images, res_labels, 20);
    }
}

int main(int argc, char** argv)
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {

        af::deviceset(device);
        af::info();
        naive_bayes_demo(console);

    } catch (af::exception &ae) {
        std::cout << ae.what() << std::endl;
    }

}
