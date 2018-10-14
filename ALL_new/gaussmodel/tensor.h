#ifndef TENSOR_H
#define TENSOR_H
#include <iostream>
#include <vector>
#include <memory>

class Tensor
{
    friend Tensor stack_tensors(const std::vector<Tensor> &ts);
//二维数组（比三维更容易处理，实现）
/**
  *出现了多处重新分配内存靠背元素的操作，如果对Tensor索引访问的需求不高的话，可以用链表代替数组的方式（连续内存）实现。
  */
public:

    typedef double* iterator;
    typedef const double* const_iterator;

    Tensor();
    ~Tensor();
	Tensor(const Tensor & obj);
    Tensor(Tensor &&);
    explicit Tensor(int rows);
	Tensor(int rows, int cols);
	Tensor(int rows, int cols, const std::initializer_list<double> &list);

    //private数据的getter函数
    const int rows() const;
    const int cols() const;
    //运算符
    double & operator()(int i, int j);
    const double & operator()(int i, int j) const;
    Tensor & operator = (const Tensor& obj) &;
    Tensor & operator = (Tensor &&) &;
    //类似于matlab切片操作，不能对切片数组进行赋值操作
	Tensor slice(int start_row, int end_row, int start_col, int end_col) const;
	//返回索引指定行位置的行向量,(1,n)
	Tensor row_vector(int pos) const;
	//返回索引指定列位置的列向量，（n,1）
	Tensor col_vector(int pos) const;
	//non-sense
//	Tensor concatenate(const vector<Tensor> & vec, int axis) const;
	//mean
	Tensor mean(int axis) const;
	void vector_arg_max(double &val, int &pos) const;
    //扩充Tensor，重新分配内存
    Tensor & extend(const Tensor &obj);


	iterator begin();
    const_iterator begin() const;
	iterator end();
	const_iterator end() const;
    const bool isEmpty() const;

private:
    
    int row;
    int col;
	std::unique_ptr<double[]> buffer;

};

void print(const Tensor & obj);

#endif // TENSOR_H
