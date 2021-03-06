﻿#include "tensor.h"
#include <algorithm>
#include <iterator>
#include <numeric>
#include <assert.h>

Tensor::Tensor()
{
    row=0;
    col=0;
}

Tensor::Tensor(const Tensor & obj) :
	row(obj.row),
	col(obj.col),
	buffer(new double[row*col])
{
    std::copy(obj.begin(), obj.end(), buffer.get());
}
Tensor::Tensor(Tensor && obj)
    : row(obj.row)
    , col(obj.col)
    , buffer(obj.buffer.release())
{
    obj.row = 0;
    obj.col = 0;
}

Tensor::Tensor(int rows) :
	row(rows),
	col(1),
	buffer(new double[row*col])
{
}
Tensor::Tensor(int rows, int cols) :
	row(rows),
	col(cols),
	buffer(new double[row*col])
{
}
Tensor::Tensor(int rows, int cols, const std::initializer_list<double> &list) :
	row(rows),
	col(cols),
	buffer(new double[row*col])
{
	std::copy(list.begin(), list.end(), buffer.get());
}

Tensor::~Tensor()
{
    row=0;
    col=0;
    //free();
}
const int Tensor::rows() const
{
    return row;
}
const int Tensor::cols() const
{
    return col;
}
double & Tensor::operator()(int i, int j)
{
    return buffer[i*col + j];
}
const double & Tensor::operator()(int i, int j) const
{
    return buffer[i*col + j];
}

Tensor & Tensor::operator = (const Tensor& obj) &
{
    if (this != &obj)
    {
        row = obj.rows();
        col = obj.cols();

		buffer.release();
		buffer.reset(new double[row*col]);
        std::copy(obj.begin(), obj.end(), buffer.get());
    }
	
	return *this;
}
Tensor & Tensor::operator =(Tensor && obj) &
{
    if(this != &obj)
    {
		buffer.reset(obj.buffer.release());
        row = obj.rows();
        col = obj.cols();

		obj.buffer.reset(nullptr);
        obj.row = 0;
        obj.col = 0;
    }
    return *this;
}
/*
*tensor.slice(a,b,c,d)取得元素为tensor[a]:tensor[b],b位置元素取不到，共取b-a个元素
*参数取默认值-1表示取到最结尾tensor.slice(0,-1,0,-1)取全部元素
*/
Tensor Tensor::slice(int start_row=0, int end_row=-1, int start_col=0, int end_col=-1) const
{
	if (start_row<0 || start_col<0 || end_row<-1 || end_col<-1 ||
		(start_row > end_row && end_row != -1) || (start_col > end_col && end_col != -1) ||
		start_row > this->row || start_col > this->col)
	{
		std::cout << start_row << " " << end_row << " " << start_col << " " << end_col << std::endl;
        assert(false && "Tensor::slice: illegal paramesters");
	}
	if (end_row == -1)	end_row = this->row;
	if (end_col == -1)	end_col = this->col;

	Tensor obj(end_row - start_row, end_col - start_col);

	for (int m = 0, i = start_row; i < end_row; i++, m++)
	{
		for (int n=0, j = start_col; j < end_col; j++, n++)
		{
			obj(m, n) = (*this)(i, j);
		}
	}
	return obj;
}
Tensor Tensor::row_vector(int pos) const
{
    assert(!(pos <0 || pos >= row) && "Tensor::row_vector: illegal paramesters");
	Tensor ret(1, col);
    std::copy(buffer.get() + pos*col, buffer.get() + pos*col + col, ret.begin());

	return ret;
}

Tensor Tensor::col_vector(int pos) const
{
    assert(!(pos <0 || pos >= col) && "Tensor::col_vector: illegal paramesters");
	Tensor ret(row, 1);
	for (int i = 0; i < row; i++)
		ret(i, 0) = (*this)(i, pos);

	return ret;
}

void print(const Tensor & obj)
{
    int row = obj.rows();
    int col = obj.cols();

    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            std::cout<<obj(i,j)<<",";
        }
        std::cout<<std::endl;
    }
}
/*
求均值，返回Tensor
axis=0， 行方向
axis=1， 列方向，对一个行向量内元素平均得到列向量
*/
Tensor Tensor::mean(int axis) const
{
	double sum = 0;
	if (axis == 0)
	{
		Tensor obj(1, this->col);
		
		for (int i = 0; i < this->col; i++)
		{
			sum = 0;
			for (int j = 0; j < this->row; j++)
				sum += (*this)(j, i);
			obj(0, i) = sum / (this->row);
		}
		return obj;
	}else{
		Tensor obj(row, 1);

		for (int i = 0; i < row; i++)
		{
            sum = std::accumulate(begin()+i*cols(), begin()+(i+1)*cols(), 0.0);
			obj(i, 0) = sum / col;
		}
		return obj;
	}
}
/*
对一维向量求最大值以及在所在方向上的位置，注意只能是一维矩阵，某个轴上的维度为1。
val，pos均为外部变量。
*/
void Tensor::vector_arg_max(double &val, int &pos) const
{
	val = (*this)(0,0);
	pos = 0;
	for (int i = 0; i < row;i++)
	for (int j = 0; j < col; j++)
	{
		if ((*this)(i, j) > val)
		{
			pos = i*col + j;
			val = (*this)(i, j);
		}
	}
}
/*
* 扩充Tensor，重新分配内存,扩充的tensor列方向必须相同
* (n1, m), (n2, m)
*/
Tensor& Tensor::extend(const Tensor &obj)
{
    if(obj.isEmpty())
        return *this;

    if(this->isEmpty())
    {
        *this = obj;
        return *this;
    }
    if(obj.cols() != this->cols())
        std::cout <<"Tensor::extend "<<obj.cols() <<" "<< this->cols()<< std::endl;
    assert(obj.cols() == this->cols() &&
               "Tensor::extend" &&
               "different cols shape!");
    //这个笛梵必须保证容器内存是连续的，考虑能不能用移动替换掉拷贝
	std::unique_ptr<double[]> buf_t(new double[(this->rows() + obj.rows()) * this->cols()]);
	std::copy(begin(), end(), buf_t.get());
	std::copy(obj.begin(), obj.end(), buf_t.get() + col * row);
    row += obj.rows();
	buffer.reset(buf_t.release());

    return *this;
}
Tensor::iterator Tensor::begin()
{
	return buffer.get();
}
Tensor::const_iterator Tensor::begin() const
{
	return buffer.get();
}
Tensor::iterator Tensor::end()
{
	return buffer.get() + row * col;
}
Tensor::const_iterator Tensor::end() const
{
	return buffer.get() + row * col;
}
const bool Tensor::isEmpty() const
{
	return begin() == end();
}

/***************************common utils************/
Tensor stack_tensors(const std::vector<Tensor> &ts)
{
    assert(ts.size()>0 && "stack_tensors: tensors vector size must larger than zero!");

    int size = 0;

    auto it = ts.begin();
    int last_col = (*it).cols();
    for(;it != ts.end(); it++)
    {
        size += (*it).rows() * (*it).cols();
        assert((*it).cols() == last_col &&  "Tensor::extends " && "different cols shape!");
    }
	std::unique_ptr<double[]> buf_t(new double[size]);

    auto p = buf_t.get();
    for(it = ts.begin(); it != ts.end(); it++)
    {
        p = std::copy((*it).begin(), (*it).end(), p);
    }

    Tensor ret;

    ret.buffer.reset(buf_t.release());
    ret.col = last_col;
    ret.row = size / last_col;

    return ret;
}
