
#include "tensor.h"

int main(int argc, char *argv[])
{


    Tensor T(2,3,2);
    for(int i=0;i<T.rows();i++)
    {
        for(int j=0;j<T.cols();j++)
        {
            for(int k=0;k<T.chns();k++)
                T(i,j,k) = i*T.cols() + j*T.chns() + k;
        }
    }
    print(T);

	Tensor tt = T.at(1, 1);
	print(tt);
	getchar();
	return 0;
}
