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

// Calculate all the distances from testing set to training set
array distance(array train, array test)
{
    const int feat_len = train.dims(1);
    const int num_train = train.dims(0);
    const int num_test  =  test.dims(0);
    array dist = constant(0, num_train, num_test);

    // Iterate over each attribute
    for (int ii = 0; ii < feat_len; ii++) {

        // Get a attribute vectors
        array train_i = train(span, ii);
        array test_i  = test (span, ii).T();

        // Tile the vectors to generate matrices
        array train_tiled = tile(train_i, 1,   num_test);
        array test_tiled  = tile( test_i, num_train, 1 );

        // Add the distance for this attribute
        dist = dist + abs(train_tiled - test_tiled);
    }

    return dist;
}

array knn(array &train_feats, array &test_feats, array &train_labels)
{
    // Find distances between training and testing sets
    array dist = distance(train_feats, test_feats);

    // Find the neighbor producing the minimum distance
    array val, idx;
    min(val, idx, dist);

    // Return the labels
    return train_labels(idx.as(f32)).T();
}

void knn_demo(bool console)
{
    array train_images, train_labels;
    array test_images, test_labels;
    int num_train, num_test, num_classes;

    // Load mnist data
    setup_mnist<false>(&num_classes, &num_train, &num_test,
                       train_images, test_images,
                       train_labels, test_labels);

    int feature_length = train_images.elements() / num_train;
    array train_feats = moddims(train_images, feature_length, num_train).T();
    array test_feats  = moddims(test_images , feature_length, num_test ).T();

    // Get the predicted results
    array res_labels = knn(train_feats, test_feats, train_labels);

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
        knn_demo(console);

    } catch (af::exception &ae) {
        std::cout << ae.what() << std::endl;
    }

}
