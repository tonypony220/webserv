#include <iostream>
#include <vector>
#include <map>
using namespace std;
int main () {
	char buff[40];
	vector<char> v;
//	v.push_back(20);
	memset(buff, 42, 40);
	v.insert(v.end(), buff, buff + 40);
	v.insert(v.end(), buff, buff + 40);
	v.erase(v.begin(), v.begin()+40);
	cout << v.size();
}
