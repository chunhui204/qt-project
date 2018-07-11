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

const double RSQRT2PI = 0.39894228040143267793994605993438;//����2pi��֮һ
const double RSQRTPI = 0.56418958354775628694807945156079;//����pi��֮һ
Paramester & gausianTrain(const vector<Tensor> &, float, float, vector<int>);
class Algorithm
{
public:
    Algorithm();
    ~Algorithm();

};

#endif // ALGORITHM_H
