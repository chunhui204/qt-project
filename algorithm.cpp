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
	return 0.5 + 0.5*erf((x - miu)*RSQRT2 / sigma);
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
*根据已知数据找出每类数据的每个维度特征的均值和标准差
*@input params
	trainData : 训练数据，三个维度一次是样本数量、特征维度、类别维度
	cdfThreshold: 设定样本cdf的阈值 令cdf小于cdfgate和大于1-cdfgate的点的pdf置零     例如取0.03
	probThreshold： 设定总的概率的容错率  将会有probthreshold的正样本可能被分为负样本，设为0则有可能提高误检率 例如取0.01
	group ： 向量，用来划定哪几维的特征是一个组合,如[5 3 7]表示从左到右的特征共有三组，维度依次为5维 3维
			 7维，特征维度为5+3+7=15维
*@output params:
	一定要传入外部对象，非临时对象
	featMean: 特征每个维度的均值 每行为一类数据，每列为特征的一个维度
	featStd: 特征每个维度的标准差
	gate: 概率（pdf）均值的门限阈值 行向量
*
*/
Paramester gausianTrain(const vector<Tensor> &trainData, double cdTthreshold, double probThreshold, vector<int> group)
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
			
			normfit(data.slice_row(0, -1, dim), miu, sigma);
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
/*
gate shape: (classNum, 1)
*/
Tensor gausianTest(const Tensor & testdata, const Tensor & featmean, const Tensor & featstan, const Tensor & gate, double cdfthreshold, vector<int> group)
{
	if (testdata.rows() < 1){
		throw "训练数据为空";
	}

	int classNum = featmean.rows();
	int dataNum = testdata.rows();
	int featDim = testdata.cols();

	Tensor resultpb(dataNum, classNum);
	Tensor result(dataNum, 1);
	for (int cn = 0; cn< classNum; cn++)  //按每类数据测试
	{
		Tensor probp(dataNum, featDim);
		//按每个维度测试
		for (int dim = 0; dim < featDim; dim++)
		{
			vector<int> indexs;//不在阈值范围内的sample索引
			for (int num = 0; num < dataNum; num++)
			{
				double cdf = normcdf(featmean(cn, dim), featstan(cn, dim), testdata(num, dim)); //calc cdf

				//概率密度在概率分布两端的点被设置为0，最后以概率密度为基准 
				if (cdf < cdfthreshold || cdf > 1 - cdfthreshold)
					probp(num, dim) = 0;
				else
					probp(num, dim) = normpdf(featmean(cn, dim), featstan(cn, dim), testdata(num, dim));//calc pdf
			}
		}
		//print(probp.slice(0, -1, 0,-1));
		//对每个特征组内的概率求均值，作为该组所有特征维度的概率值
		Tensor probpt(dataNum, group.size());
		int last_index = 0;
		for (int g = 0; g < group.size(); g++)
		{
			Tensor temp = probp.slice(0, -1, last_index, last_index+group[g]).mean(1);
			
			for (int num = 0; num < dataNum; num++)
				probpt(num, g) = temp(num, 0);
			//refresh last_index
			last_index = group[g];
		}
		cout << "probpt:" << endl;
		print(probpt);
		Tensor meanProbpt = probpt.mean(1);
		for (int num = 0; num < dataNum; num++)
		{
			if (meanProbpt(num, 0) < gate(cn, 0))
				meanProbpt(num, 0) = 0;
			//属于每一类的pdf均值，每行为一个样本，每列为一类
			resultpb(num, cn) = meanProbpt(num, 0);
		}
		
	}

	cout << "resultpb" << endl;
	print(resultpb);
	//当一个样本属于所有类别的概率均为0（或最大值为0）时认为是异常点
	for (int num = 0; num < dataNum; num++)
	{
		double maxval = 0;
		int maxpos = 0;
		resultpb.slice_col(num, 0, -1).vector_max(maxval, maxpos);
		if (maxval == 0)
			result(num, 0) = 0;
		else
			result(num, 0) = maxpos;
	}
	return result;
}
