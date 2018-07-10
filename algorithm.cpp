#include "algorithm.h"
#include <math.h>

Algorithm::Algorithm()
{

}
Algorithm::~Algorithm()
{

}
double normpdf(const double mean, const double std, const double x)
{
	const double RSQRT2PI = 0.39894228040143267793994605993438;//根下2pi分之一
	return exp(-(x - mean)*(x - mean)*0.5 / (std*std))*RSQRT2PI / std;
}
double normcdf(const double mean, const double std, const double x)
{
	double sum = 0;
	normpdf(xt, mu, sigma)* x(i) / resolution
}
/*
*正态分布拟合
*data必须是一维列向量(n,1)
*mean，std是外部变量传入，返回拟合高斯的参数
*/
void normfit(const Tensor & data, double & mean, double & std)
{
	//mean
	double sum = 0;
	int N = data.rows()*data.cols();
	for (int i = 0; i < data.rows(); i++)
	for (int j = 0; j < data.cols(); j++)
		sum += data(i, j);
	mean = sum / N;
	//std
	sum = 0;
	for (int i = 0; i < data.rows(); i++)
	for (int j = 0; j < data.cols(); j++)
		sum += (data(i, j) - mean)*(data(i, j) - mean);
	std = sqrt(sum / (N - 1));
}
/*
*模型训练
*data是3维向量，（sample_num,feature_dim, class_num）
*
*/
Paramester & gausianTrain(const vector<Tensor> &trainData, double cdTthreshold, double probThreshold, vector<int> group)
{
	if (trainData.size() < 1){
		throw "训练数据为空";
	}

	int classNum = trainData.size();
    int dataNum = trainData[0].rows();
    int featDim = trainData[0].cols();
    
	Paramester params = { new Tensor(classNum, featDim), new Tensor(classNum, featDim), new Tensor(classNum) };

    for(int cn = 0;cn< classNum; cn++)  //处理第cn类数据
    {
		Tensor data = trainData[cn];
        //根据已知数据找出每类数据的每个维度特征的均值和标准差
		for (int dim = 0; dim < featDim; dim++)
		{
			double mean = 0;
			double std = 0;
			normfit(data.slice(0, -1, dim), mean, std);
			(*(params.featureMean))(cn, dim) = mean;
			(*(params.featureStd))(cn, dim) = std;
		}
    }

    return params;
}
