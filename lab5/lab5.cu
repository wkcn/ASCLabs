#include <iostream>
#include <fstream>
#include <ctime>
#include <cuda_runtime.h>

using namespace std;

const int N = 400;
const int P = 600;
const int M = 200;

float A[N][P];
float B[P][M];
float C[N][M];

float *A_D,*B_D,*C_D;

void Init_Data(){
	//Mat A
	for (int i = 0;i < N;++i){
		for (int j = 0;j < P;++j){
			A[i][j] = (j+1) * 1.f / (N * (i+1));
		}
	}
	//Mat B
	for (int i = 0;i < P ;++i){
		for (int j = 0;j  < M;++j){
			B[i][j] = M * 1.0 / ((i+1) * (j+1));
		}
	}
	//Copy data to cuda
	cudaMalloc((void **)&A_D, sizeof(float) * N * P);
	cudaMalloc((void **)&B_D, sizeof(float) * P * M);
	cudaMalloc((void **)&C_D, sizeof(float) * N * M);
	cudaMemcpy(A_D, (void*)A, sizeof(float) * N * P, cudaMemcpyHostToDevice);
	cudaMemcpy(B_D, (void*)B, sizeof(float) * P * M, cudaMemcpyHostToDevice);
}

__global__ void MatMul(float *A_D, float *B_D, float *C_D){
	int i = threadIdx.x + blockIdx.x * blockDim.x;
	int j = threadIdx.y + blockIdx.y * blockDim.y;
	if (i >= N || j >= M)return;
	
	float v = 0;
	for (int k = 0;k < P;++k){
		v += A_D[i * P + k] * B_D[k * M + j];
 	}
	C_D[i * M + j] = v;
	
} 

void Output(){
	cudaMemcpy((void*)C, C_D, sizeof(float) * N * M, cudaMemcpyDeviceToHost); 
	ofstream fout("cudaresult.txt");
	for (int i = 0;i < N;++i){
		for (int j = 0;j < M;++j){
			if (j != 0){
				fout << " ";
			}
			fout << C[i][j];
		}
		fout << endl;
	}
}

int main(){
	Init_Data();
	dim3 dimGrid(20, 20);
	//dim3 dimBlock(N,M);
	//dim3 dimBlock((N+31)/32*32,(M+31)/32*32);
	dim3 dimBlock(20,10);
	clock_t t = clock();
	MatMul<<<dimGrid, dimBlock>>>(A_D, B_D, C_D);
	cout << "Cuda Used Time: "<< double((clock() - t)*1.0/CLOCKS_PER_SEC) << endl;
	Output();

	//Release Source
	cudaFree(A_D);
	cudaFree(B_D);
	cudaFree(C_D);
	return 0;
}
