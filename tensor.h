#ifndef TENSOR_H
#define TENSOR_H
#include <iostream>
#include <vector>

using namespace std;

class Tensor
{
//二维数组（比三维更容易处理，实现）
public:
    Tensor();
    ~Tensor();
	Tensor(const Tensor & obj);
	Tensor(int rows);
    Tensor(int rows, int cols);

    //private数据的getter函数
    int rows() const;
    int cols() const;
    //运算符
    double & operator()(int i, int j) const;
	Tensor & operator = (const Tensor& obj);
    //类似于matlab切片操作，不能对切片数组进行赋值操作
	Tensor slice(int pos, int start_col, int end_col) const;
	Tensor slice(int start_row, int end_row, int pos) const;
	Tensor slice(int start_row, int end_row, int start_col, int end_col) const;
	//non-sense
	Tensor concatenate(const vector<Tensor> & vec, int axis) const;
	//mean
	Tensor mean(int axis) const;
private:
    double * buffer;
    int row;
    int col;

};


void print(const Tensor & obj);

#endif // TENSOR_H
