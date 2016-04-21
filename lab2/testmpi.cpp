#include <iostream>
#include <cmath>
#include "mpi.h"

//function
double f(double x){
	return sin(x);
}
double a = 0; // 下限
double b = 100; // 上限
double n = 10000; // 采样点数
//求f(x)从a到b的积分

using namespace std;
int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	cout << "Core: " << rank << " of " << size << endl;
	//计算
	double h = (b - a) / n;
	double subsum = 0;
	for (int i = rank;i < n;i += size){
		double x = i * h;
		subsum += 0.5 * h * (f(x) + f(x+h));
	}
	//通信， 合并结果
	double result = 0;
	if (rank == 0){
		result += subsum;// 首先加上自身计算的值
		//得到子进程的计算结果
		for (int i = 1;i < size;++i){
			MPI_Recv(&subsum, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			result += subsum;
		}
	}else{
		MPI_Send(&subsum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD); 
	}

	if (rank == 0){
		//使用串行计算
		double sresult = 0;
		for (int i = 0;i < n;++i){
			double x = i * h;
			sresult += 0.5 * h * (f(x) + f(x+h));
		}
		cout << "Parallel Computation: The result is " << result << endl;
		cout << "Serial Computation: The result is " << result << endl;
	}

	MPI_Finalize();
	return 0;
}
