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

Paramester & gausianTrain(const vector<Tensor> &, float, float, vector<int>);
class Algorithm
{
public:
    Algorithm();
    ~Algorithm();

};

#endif // ALGORITHM_H
