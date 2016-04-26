#include <iostream>
#include <algorithm>
#include <vector>
#include "mpi.h"
using namespace std;

const int dataCount = 18;
int data[dataCount] = {48,39,6,72,91,14,69,40,89,61,12,21,84,58,32,33,72,20};

/*
 MPI_ALLTOALLV(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm)
 IN  sendbuf     发送消息缓冲区的起始地址(可变)
 IN  sendcounts  长度为组大小的整型数组, 存放着发送给每个进程的数据
                 个数
 IN  sdispls     长度为组大小的整型数组,每个入口j存放着相对于sendbuf
                 的位移,此位移处存放着输出到进程j的数据
 IN  sendtype    发送消息缓冲区中的数据类型(句柄)
 OUT recvbuf     接收消息缓冲区的起始地址(可变)
 IN  recvcounts  长度为组大小的整型数组, 存放着从每个进程中接收的元
                 素个数(整型)
 IN  rdispls     长度为组大小的整型数组,每个入口i存放着相对于recvbuf
                 的位移,此位移处存放着从进程i接收的数据
 IN  recvtype    接收消息缓冲区的数据类型(句柄)
 IN  comm        通信子(句柄)
*/

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	double startTime = MPI_Wtime();

	if (!rank){
		cout << "#Progress: " << size << endl;
		cout << "Unsorted: ";
		bool first = true;
		for (int i = 0;i < dataCount;++i){
			if(!first)cout << ", ";
			first = false;
			cout << data[i];
		}
		cout << endl;
	}
	

	//cout << "Progress: " << rank << " of " << size << endl;
	//得到本地数据
	int u = dataCount / size;
	//[left, right)
	int left = u * rank;
	int right = u * (rank + 1);
	if (rank == size - 1)right = dataCount;
	//本地数据排序
	sort(data + left, data + right);
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

	int *fp = new int [size - 1];

	if (rank == 0){
		int m = size * size;
		sort(privots, privots + m);
		int y = m / size;
		for (int i = 0;i < size-1; ++i){
			fp[i] = privots[(i+1) * y];
		}
	}

	//1 -> N 散发最终样本, 即主元
	MPI_Bcast(fp, size-1, MPI_INT, 0, MPI_COMM_WORLD);

	int *lefts = new int[size];
	int *rights = new int[size];
	int *sendlens = new int[size];

	for (int i = 0;i < size-1;++i){
		rights[i] = upper_bound(data + left, data + right, fp[i]) - (data);
		lefts[i + 1] = rights[i];
	}
	lefts[0] = left;
	rights[size-1] = right;

	for (int i = 0;i < size;++i){
		//通知第i个进程, 自己要传播多长的数组
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
	MPI_Alltoallv(data, sendlens, lefts, MPI_INT, recvbuf, recvlens, recvoff ,MPI_INT, MPI_COMM_WORLD);
	//对最终的本地数据进行排序
	//sort(recvbuf,recvbuf + recvcount);	

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

	//汇集结果
	//发送各个进程所含数据的大小
	int *counts = new int [size];
	MPI_Gather(&recvcount, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int *displs;
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
	MPI_Gatherv(data, recvcount, MPI_INT, res, counts, displs, MPI_INT,  0, MPI_COMM_WORLD); 

	//输出结果
	if (!rank){
		cout << "Sorted: ";
		bool first = true;
		for (int i = 0;i < dataCount;++i){
			if(!first)cout << ", ";
			first = false;
			cout << res[i];
		}
		cout << endl;
		double endTime = MPI_Wtime();
		cout << "Used Time: " << endTime - startTime << endl;
	}

	//一定要加!
	MPI_Finalize();

	return 0;
}
