#ifndef ALGORITHM_H
#define ALGORITHM_H
#include "tensor.h"
#include <vector>

struct Paramester
{
   Tensor*  featureMean;
   Tensor*  featureStd;
   Tensor*  gate;
};

const double RSQRT2PI = 0.39894228040143267793994605993438;//根下2pi分之一
const double RSQRT2 = 0.70710678118654752440084436210485;//根下2分之一

double normpdf(const double miu, const double sigma, const double x);
double normcdf(const double miu, const double sigma, const double x);
//train暂时不能用
Paramester gausianTrain(const vector<Tensor> &, float, float, vector<int>);
Tensor gausianTest(const Tensor & testdata, const Tensor & featmean, const Tensor & featstan, const Tensor & gate, double cdfthreshold, vector<int> group);
class Algorithm
{
public:
    Algorithm();
    ~Algorithm();

};

#endif // ALGORITHM_H
