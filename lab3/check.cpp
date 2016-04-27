#include <iostream>
#include <fstream>
#include <map>
#include <vector>
using namespace std;

bool check(){
	ifstream fs("sorted.txt"), fu("unsorted.txt");
	if (fs.fail() || fu.fail()){
		cout << "文件打开失败!" << endl;
		return false;
	}
	vector<int> ds,du;
	int k;
	fs >> k;
	while(!fs.eof()){ds.push_back(k);fs >> k;}
	fu >> k;
	while(!fu.eof()){du.push_back(k);fu >> k;}
	map<int,int> hs,hu;
	for (size_t i = 0;i < ds.size();++i){
		if (i > 0){
			if (ds[i - 1] > ds[i]){
				cout << "存在逆序对 (" << ds[i-1] << ", " << ds[i] << ")" << endl; 
				return false;
			}
		}
		if (!hs.count(ds[i]))hs[ds[i]] = 0;
		++hs[ds[i]];
	}	
	for (size_t i = 0;i < du.size();++i){
		if (!hu.count(du[i]))hu[du[i]] = 0;
		++hu[du[i]];
	}
	for (auto h = hs.begin();h != hs.end();++h){
		if (hu.count(h->first) == 0 || hu[h->first] != h->second){
			cout << "存在数量不匹配" << h->first << "| " << h->second << endl;
			return false;
		}
	}
	return true;
}

int main(){
	if (check()){
		cout << "Sort Success!" << endl;
	}else{
		cout << "Fail!" << endl;
	}
	return 0;
}
