#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;
int main(){
	srand(time(0));
	cout << "请输入你想生成的随机数个数: ";
	int k;
	cin >> k;
	ofstream fout("unsorted.txt");
	for (int i = 0;i < k;++i){
		fout << rand() << " ";
	}
	fout.close();
	return 0;
}
