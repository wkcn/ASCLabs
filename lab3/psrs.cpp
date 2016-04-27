#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include "mpi.h"
using namespace std;

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (!rank){
		cout << "#Progress: " << size << endl;
	}

	ifstream fin("unsorted.txt");
	vector<int> data;
	int temp;
	fin >> temp;
	while(!fin.eof()){
		data.push_back(temp);
		fin >> temp;
	}
	int dataCount = data.size();

	double startTime = MPI_Wtime();


	//cout << "Progress: " << rank << " of " << size << endl;
	//得到本地数据
	int u = dataCount / size;
	//[left, right)
	int left = u * rank;
	int right = u * (rank + 1);
	if (rank == size - 1)right = dataCount;
	//本地数据排序
	sort(data.data() + left, data.data() + right);
	//等间隔采样
	int n = right - left; // 本地数据大小
	int j = n / size; // 间隔
	int *send_privots = new int[size];
	//间隔采样
	for (int i = 0;i < size;++i){
		send_privots[i] = data[left + j * i];
	}
	//收集采样, 结果放在0号进程的privots中
	int *privots = new int [size * size];
	// N -> 1
	MPI_Gather(send_privots, size, MPI_INT, privots, size, MPI_INT, 0, MPI_COMM_WORLD);
	delete []send_privots;

	int *fp = new int [size - 1];

	if (rank == 0){
		int m = size * size;
		sort(privots, privots + m);
		int y = m / size;
		for (int i = 0;i < size-1; ++i){
			fp[i] = privots[(i+1) * y];
		}
	}
	delete []privots;

	//1 -> N 散发最终样本, 即主元
	MPI_Bcast(fp, size-1, MPI_INT, 0, MPI_COMM_WORLD);

	int *lefts = new int[size];
	int *rights = new int[size];
	int *sendlens = new int[size];

	for (int i = 0;i < size-1;++i){
		//upper_bound: 找出第一个键值大于key的元素
		rights[i] = upper_bound(data.data() + left, data.data() + right, fp[i]) - (data.data());
		lefts[i + 1] = rights[i];
		//fp[i-1] < [left,right) <= fp[i]
	}

	lefts[0] = left;
	rights[size-1] = right;

	for (int i = 0;i < size;++i){
		//计算发送多长的数组
		int len = rights[i] - lefts[i];
		sendlens[i] = len;
	}

	//接收长度
	int *recvlens = new int[size];
	int *recvoff = new int[size];
	int recvcount = 0;

	//1 为 发送 或 从某个进程接收 的元素个数
	MPI_Alltoall(sendlens, 1, MPI_INT, recvlens, 1, MPI_INT, MPI_COMM_WORLD); 

	for (int i = 0;i < size; ++i){
		recvoff[i] = recvcount;
		recvcount += recvlens[i];
	}

	int *recvbuf = new int[recvcount];

	//Alltoallv
	MPI_Alltoallv(data.data(), sendlens, lefts, MPI_INT, recvbuf, recvlens, recvoff ,MPI_INT, MPI_COMM_WORLD);
	delete []lefts;
	delete []rights;
	delete []sendlens;
	//对最终的本地数据进行排序

	//多路归并, 覆盖掉data, 反正没用了
	int *is = new int [size];
	for (int i = 0;i < size;++i){
		is[i] = 0;
	}

	for (int u = 0;u < recvcount;++u){
		bool valid = false;
		int minnum = 0;
		int sid = 0;
		for (int i = 0;i < size;++i){
			int j = is[i];
			if (j < recvlens[i]){
				//注意比较大小的方向!
				if (!valid || recvbuf[recvoff[i] + j] < minnum){
					minnum = recvbuf[recvoff[i] + j];
					valid = true;
					sid = i;
				}
			}
		}
		data[u] = minnum;
		++is[sid];
	}
	delete []recvbuf;
	delete []recvoff;
	delete []recvlens;
	delete []is;

	//汇集结果
	//发送各个进程所含数据的大小给0号进程
	int *counts = new int [size];
	MPI_Gather(&recvcount, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int *displs = 0;
	if (rank == 0){
		displs = new int [size];
		int u = 0;
		for (int i = 0;i < size;++i){
			displs[i] = u;
			u += counts[i];
		}
	}

	//得到各个进程的排序结果
	int *res = new int [dataCount];
	MPI_Gatherv(data.data(), recvcount, MPI_INT, res, counts, displs, MPI_INT,  0, MPI_COMM_WORLD); 
	//delete []counts;
	if (!displs) delete []displs;

	//输出结果
	if (!rank){
		double endTime = MPI_Wtime();
		cout << "Used Time: " << endTime - startTime << endl;
		ofstream fout("sorted.txt");
		for (int i = 0;i < dataCount;++i){
			fout << res[i] << " "; 
		}
		fout.close();
		delete []res;
	}

	//一定要加!
	MPI_Finalize();

	return 0;
}
