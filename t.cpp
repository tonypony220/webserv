#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
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
	string s;
	/* int size = 400000; */
	/* s.resize(size); */
	/* /1* s.resize(40); *1/ */
	/* for (int i = 0; i < size ; i++) { */
	/* 	if (s[i] != 0) */
	/* 	cout << int(s[i]); */
	/* } */
	/* cout << string(NULL); */


//	setenv(std::string("PATH").c_str(), "hello", 1 );
//	if(const char* env_p = std::getenv("PATH"))
//		std::cout << "Your PATH is: " << env_p << '\n';


	const char *args[3] = {"python3",
						   "/Users/mehtel/coding/webserv/py.py",
										  0};
	execvp(args[0], (char* const*)args);
	perror("");

}
