#include "tensor.h"

Tensor::Tensor()
{
    row=0;
    col=0;
    buffer=NULL;
}

Tensor::Tensor(const Tensor & obj)
{
	row = obj.rows();
	col = obj.cols();

	buffer = new double[this->row * this->col];

	for (int i = 0; i < row; i++)
	for (int j = 0; j < col; j++)
	{
		*(buffer + i*col + j) = *(obj.buffer + i*col + j);
	}

}
Tensor::~Tensor()
{
    row=0;
    col=0;
    delete[] buffer;
    buffer = NULL;
}
Tensor::Tensor(int rows)
{
	this->row = rows;
	this->col = 1;
	buffer = new double[this->row * this->col]();
}
Tensor::Tensor(int rows, int cols)
{
    this->row = rows;
    this->col = cols;
    buffer = new double[this->row * this->col]();
}

int Tensor::rows() const
{
    return row;
}
int Tensor::cols() const
{
    return col;
}
double & Tensor::operator()(int i, int j) const
{
    return buffer[i*col + j];
}


Tensor & Tensor::operator = (const Tensor& obj)
{
    if (this != &obj)
    {
        row = obj.rows();
        col = obj.cols();

        if(this != NULL)
            delete [] buffer;
        buffer = new double[this->row * this->col];

        for(int i=0;i<row;i++)
            for(int j=0;j<col;j++)
            {
				*(buffer + i*col + j) = *(obj.buffer + i*obj.cols() + j);
            }
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
		throw "参数不合法";
	}
	if (end_row == -1)	end_row = this->row;
	if (end_col == -1)	end_col = this->col;

	Tensor obj(end_row - start_row, end_col - start_col);

	for (int m = 0,int i = start_row; i < end_row; i++, m++)
	{
		for (int n=0, int j = start_col; j < end_col; j++, n++)
		{
			obj(m, n) = (*this)(i, j);
		}
	}
	return obj;
}
Tensor Tensor::slice(int pos, int start_col, int end_col) const
{
	if (pos<0 || start_col<0 || end_col<-1 ||
		(start_col > end_col && end_col != -1) ||
		pos >= this->row || start_col > this->col)
	{
		throw "参数不合法";
	}
	if (end_col == -1)	end_col = this->col;

	Tensor obj(1, end_col - start_col);

	for (int n = 0, int j = start_col; j < end_col; j++, n++)
	{
		obj(0, n) = (*this)(pos, j);
	}
	return obj;
}
Tensor Tensor::slice(int start_row, int end_row, int pos) const
{
	if (start_row<0 || pos<0 || end_row<-1 ||
		(start_row > end_row && end_row != -1) ||
		start_row > this->row || pos >= this->col)
	{
		throw "参数不合法";
	}
	if (end_row == -1)	end_row = this->row;

	Tensor obj(end_row - start_row, 1);

	for (int m = 0, int i = start_row; i < end_row; i++, m++)
	{
		obj(m, 0) = (*this)(i, pos);
	}
	return obj;
}
/*
将vector内的Tensor排列成一个Tensor，要求Tensor必须是向量形状（1,n） or (n,1)
axis=0,：在行方向组合，要求列向量Tensor(n,1)
axis=1,：在列方向组合，要求行向量Tensor(1，n)
*/
Tensor Tensor::concatenate(const vector<Tensor> & vec, int axis) const
{
	if (vec.size < 1 || (axis == 0 && vec[0].col != 1) || (axis == 1 && vec[0].row != 1))
	{
		throw "参数有误";
	}
	//还应该断言vector内tensor形状一致
	if (axis == 0)
	{
		Tensor obj(vec[0].row, vec.size());
		for (int i = 0; i < obj.row;i++)
		for (int j = 0; j < obj.col; j++)
			obj(i, j) = vec[j](i, 0);

		return obj;
	}
	else if (axis == 1)
	{
		Tensor obj(vec.size(), vec[0].col);
		for (int i = 0; i < obj.row; i++)
		for (int j = 0; j < obj.col; j++)
			obj(i, j) = vec[i](0, j);

		return obj;
	}
	
}
void print(const Tensor & obj)
{
    int row = obj.rows();
    int col = obj.cols();

    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            cout<<obj(i,j)<<"\t";
        }
        cout<<endl;
    }
}
/*
求均值，返回Tensor
axis=0， 行方向,是对列向量平均得到行向量
axis=1， 列方向，对每个行向量平均得到列向量
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
			sum = 0;
			for (int j = 0; j < col; j++)
				sum += (*this)(i, j);
			obj(i, 0) = sum / col;
		}
		return obj;
	}
}