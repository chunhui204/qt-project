#include "algorithm.h"
#include <math.h>
#include <algorithm>

Algorithm::Algorithm()
{

}
Algorithm::~Algorithm()
{

}
/*
reference:
[1] http://www.cplusplus.com/reference/cmath/erf/
[2] https://zh.wikipedia.org/wiki/%E6%AD%A3%E6%80%81%E5%88%86%E5%B8%83
*/
double normpdf(const double miu, const double sigma, const double x)
{
	return exp(-(x - miu)*(x - miu)*0.5 / (sigma*sigma))*RSQRT2PI / sigma;
}
double normcdf(const double miu, const double sigma, const double x)
{
	return 0.5 + 0.5*erf((x - miu)*RSQRTPI / sigma);
}
/*
*正态分布拟合
*data必须是一维列向量(n,1)
*miu，sigma是外部变量传入，返回拟合高斯的参数
*/
void normfit(const Tensor & data, double & miu, double & sigma)
{
	//miu
	double sum = 0;
	int N = data.rows()*data.cols();
	for (int i = 0; i < data.rows(); i++)
	for (int j = 0; j < data.cols(); j++)
		sum += data(i, j);
	miu = sum / N;
	//sigma
	sum = 0;
	for (int i = 0; i < data.rows(); i++)
	for (int j = 0; j < data.cols(); j++)
		sum += (data(i, j) - miu)*(data(i, j) - miu);
	sigma = sqrt(sum / (N - 1));
}
/*
*模型训练
*data是3维向量，（sample_num,feature_dim, class_num）
*
*/
Paramester & gausianTest(const vector<Tensor> &trainData, double cdTthreshold, double probThreshold, vector<int> group)
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
		Tensor probp(dataNum, featDim);
        //根据已知数据找出每类数据的每个维度特征的均值和标准差
		for (int dim = 0; dim < featDim; dim++)
		{
			double miu = 0;
			double sigma = 0;
			
			normfit(data.slice(0, -1, dim), miu, sigma);
			(*(params.featureMean))(cn, dim) = miu;
			(*(params.featureStd))(cn, dim) = sigma;
			//通过训练数据找到pdf的门限阈值
			vector<int> indexs;//不在阈值范围内的sample索引
			for (int num = 0; num < dataNum; num++)
			{
				probp(num, dim) = normpdf(miu, sigma, data(num, dim));//calc pdf
				double cdf = normcdf(miu, sigma, data(num, dim)); //calc cdf
				if (cdf < cdTthreshold || cdf > 1 - cdTthreshold)
					indexs.push_back(num);
			}
			//概率密度在概率分布两端的点被设置为0，最后以概率密度为基准 
			for (vector<int>::iterator it = indexs.begin(); it != indexs.end(); it++)
				probp(*it, dim) = 0;
		}
		//对每个特征组内的概率求均值，作为该组所有特征维度的概率值
		Tensor probpt(dataNum, group.size());
		int last_index = 0;
		for (int g = 0; g < group.size(); g++)
		{
			last_index = group[g];
			Tensor temp = probp.slice(0, -1, last_index, group[g]).mean(1);
			for (int num = 0; num < dataNum; num++)
				probpt(num, g) = temp(num, 0);
		}
		Tensor meanProbpt = probpt.mean(1);
		double probptMiu = 0; 
		double probptSigma = 0;
		normfit(meanProbpt, probptMiu, probptSigma);
    }

    return params;
}
