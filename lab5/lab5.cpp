#include <iostream>
#include <fstream>
#include <ctime>
using namespace std;

const int N = 400;
const int P = 600;
const int M = 200;

float A[N][P];
float B[P][M];
float C[N][M];

void Init_Data(){
	//Mat A
	for (int i = 0;i < N;++i){
		for (int j = 0;j < P;++j){
			A[i][j] = (j+1) * 1.f / (N * (i+1));
		}
	}
	//Mat B
	for (int i = 0;i < P;++i){
		for (int j = 0;j < M;++j){
			B[i][j] = M * 1.0 / ((i+1) * (j+1));
		}
	}
}

void MatMul(){
	for (int i = 0;i < N;++i){
		for (int j = 0;j < M;++j){
			float v = 0;
			for (int k = 0;k < P;++k){
				v += A[i][k] * B[k][j];
			}
			C[i][j] = v;
		}
	}
}

void Output(){
	ofstream fout("result.txt");
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
	clock_t t = clock();
	MatMul();
	cout << "Used Time: "<< double((clock() - t)*1.0/CLOCKS_PER_SEC) << endl;
	Output();
	return 0;
}
