#include <arrayfire.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <af/utils.h>
#include <math.h>
#include "mnist_common.h"

using namespace af;
using namespace std;

// Get accuracy of the predicted results
float accuracy(const array& predicted, const array& target)
{
    return 100 * count(round(predicted) == target) / target.elements();
}

// Activation function
array sigmoid(const array &val)
{
    return 1 / (1 + exp(-val));
}

// Derivative of the activation function
array deriv(const array &out)
{
    return out * (1 - out);
}

// Cost function
double error(const array &out,
             const array &pred)
{
    array dif = (out - pred);
    return sqrt((double)(sum<float>(dif * dif)));
}

class ann {

private:
    int m_num_layers;
    vector<array> weights;

    // Add bias input to the output from previous layer
    array add_bias(const array &in);

    vector<array> forward_propagate(const array& input);

    void back_propagate(const vector<array> signal,
                        const array &pred,
                        const double &alpha);
public:

    // Create a network with given parameters
    ann(int num_layers, vector<int> layers, double range=0.05);

    // Output after single pass of forward propagation
    array predict(const array &input);

    // Method to trian the neural net
    double train(const array &input, const array &target,
                 double alpha = 0.1, int maxiter=1000, double maxerr=0.01,
                 bool verbose = false);
};

array ann::add_bias(const array &in)
{
    // Bias input is added on top of given input
    return join(0, constant(1, 1, in.dims(1)), in);
}

vector<array> ann::forward_propagate(const array& input)
{
    // Get activations at each layer
    vector<array> signal(m_num_layers);
    signal[0] = input;

    for (int i = 0; i < m_num_layers - 1; i++) {
        array in = add_bias(signal[i]);
        array out = matmul(weights[i], in);
        signal[i + 1] = sigmoid(out);
    }

    return signal;
}

void ann::back_propagate(const vector<array> signal,
                         const array &target,
                         const double &alpha)
{

    // Get error for output layer
    array out = signal[m_num_layers  - 1];
    array err = (out - target);
    int m = target.dims(1);

    for (int i = m_num_layers - 2; i >= 0; i--) {
        array in = add_bias(signal[i]);

        array delta = deriv(out) * err;

        // Adjust weights
        array grad = -(alpha * matmul(delta, in.T())) / m;
        weights[i] += grad;

        // Input to current layer is output of previous
        out = signal[i];
        err = matmul(weights[i].T(), delta);

        // Remove the error of bias and propagate backward
        err = err(seq(1, out.dims(0)), span);
    }

}

ann::ann(int num_layers, vector<int> layers, double range) :
    m_num_layers(num_layers), weights(num_layers - 1)
{
    // Generate uniformly distributed random numbers between [-range/2,range/2]
    for (int i = 0; i < num_layers - 1; i++) {
        weights[i] = range * randu(layers[i + 1], layers[i] + 1) - range/2;
    }
}

array ann::predict(const array &input)
{
    vector<array> signal = forward_propagate(input);
    array out = signal[m_num_layers - 1];
    return out;
}

double ann::train(const array &input, const array &target,
                  double alpha, int maxiter, double maxerr, bool verbose)
{
    double err = 100;

    for (int i = 0; i < maxiter; i++) {

        // Propagate the inputs forward
        vector<array> signals = forward_propagate(input);
        array out = signals[m_num_layers - 1];

        // Check if training criterion have been met
        err = error(out, target);
        if (err < maxerr) break;


        // Print iteration count only if in verbose mode
        if (verbose && !(i%25)) {
            printf("Iteration: %4d, Err: %2.2e\n", i, err);
        }

        // Propagate the error backward
        back_propagate(signals, target, alpha);
    }

    return err;
}

int ann_demo(bool console)
{
    printf("** ArrayFire ANN Demo **\n\n");

    array train_images, test_images;
    array train_target, test_target;
    int num_classes, num_train, num_test;

    setup_mnist<true>(&num_classes, &num_train, &num_test,
                      train_images, test_images, train_target, test_target);

    int feature_size = train_images.elements() / num_train;

    // Reshape images into feature vectors
    array train_feats = moddims(train_images, feature_size, num_train);
    array test_feats  = moddims(test_images , feature_size, num_test );

    // Network parameters
    int num_layers = 3;
    vector<int> layers(num_layers);
    layers[0] = train_feats.dims(0);
    layers[1] = 16 * 16;
    layers[2] = num_classes;

    // Create network
    ann network(num_layers, layers);

    // Train network
    network.train(train_feats, train_target, 0.5, 500, 1, true);

    // Run the trained network and test accuracy.
    array train_output = network.predict(train_feats);
    array test_output  = network.predict(test_feats );

    printf("\nTraining set:\n");
    printf("Accuracy on training data: %2.2f\n",
           accuracy(train_output, train_target));

    printf("\nTest set:\n");
    printf("Accuracy on testing  data: %2.2f\n",
           accuracy(test_output , test_target ));

    if (!console) {
        // Get 20 random test images.
        display_results<true>(test_images, test_output, 20);
    }

    return 0;
}

int main(int argc, char** argv)
{
    int device = argc > 1 ? atoi(argv[1]) : 0;
    bool console = argc > 2 ? argv[2][0] == '-' : false;

    try {

        af::deviceset(device);
        af::info();
        return ann_demo(console);

    } catch (af::exception &ae) {
        std::cout << ae.what() << std::endl;
    }

}
