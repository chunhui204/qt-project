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
double normpdf(const double mean, const double std, const double x)
{
	
	return exp(-(x - mean)*(x - mean)*0.5 / (std*std))*RSQRT2PI / std;
}
double normcdf(const double mean, const double std, const double x)
{
	return 0.5 + 0.5*erf((x - mean)*RSQRTPI / std);

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
		Tensor pdfs(dataNum, featDim);
        //根据已知数据找出每类数据的每个维度特征的均值和标准差
		for (int dim = 0; dim < featDim; dim++)
		{
			double mean = 0;
			double std = 0;
			
			normfit(data.slice(0, -1, dim), mean, std);
			(*(params.featureMean))(cn, dim) = mean;
			(*(params.featureStd))(cn, dim) = std;
			//通过训练数据找到pdf的门限阈值
			vector<int> indexs;//不在阈值范围内的sample索引
			for (int num = 0; num < dataNum; num++)
			{
				pdfs(num, dim) = normpdf(mean, std, data(num, dim));//calc pdf
				double cdf = normcdf(mean, std, data(num, dim)); //calc cdf
				if (cdf < cdTthreshold || cdf > 1 - cdTthreshold)
					indexs.push_back(num);
			}
			//概率密度在概率分布两端的点被设置为0，最后以概率密度为基准 
			for (vector<int>::iterator it = indexs.begin(); it != indexs.end(); it++)
				pdfs(*it, dim) = 0;

		}
    }

    return params;
}
