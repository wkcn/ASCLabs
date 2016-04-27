#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <algorithm>
using namespace std;

int main(){
	vector<int> vs;
	int k;
	ifstream fin("unsorted.txt");
	fin >> k;
	while(!fin.eof()){
		vs.push_back(k);
		fin >> k;
	}
	clock_t t = clock();
	sort(vs.begin(),vs.end());
	cout << "Used Time: "<< (clock() - t)*1.0/CLOCKS_PER_SEC << endl;
	ofstream fout("sorted.txt");
	for (auto i:vs){
		fout << i << " ";
	}
	return 0;
}
