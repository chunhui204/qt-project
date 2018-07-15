
#include "tensor.h"
#include "algorithm.h"

int main(int argc, char *argv[])
{
	double arr[20] = { 0.3492903, -0.16396213, 0.08822347, 0.72387942, -1.74290967, -0.50930348, 0.13276021, 0.12359614, -1.56704954, 0.17401573, 0.23515456, 0.11856506, -0.51031042, 0.98646292, -1.11388244, 1.13426185, -0.77631118, 0.34916089, -0.17237246, -0.44246324 };
	Tensor testdata(5, 4);
	Tensor featmean(1, 4);
	Tensor featstan(1, 4);
	Tensor gate(1, 1);
	double cdfthreshold = 0.01;
	vector<int> group;
	/*********************************/
	for (int i = 0; i < 5;i++)
	for (int j = 0; j < 4; j++)
		testdata(i, j) = arr[i * 4 + j];

	for (int j = 0; j < 4; j++)
	{
		featmean(0, j) = 0;
		featstan(0, j) = 1;
	}
	gate(0, 0) = 0;
	group.push_back(2);
	group.push_back(2);
	/***********************************/
	Tensor result = gausianTest(testdata, featmean, featstan, gate, cdfthreshold, group);
	cout << "result" << endl;
	print(result);

	getchar();
	return 0;
}
