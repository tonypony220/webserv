#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include "Utils.hpp"
#include <vector>
#include <ctime>
#include <map>
#include <list>
using namespace std;
int main () {
	char buff[40];
	vector<char> v;
//	v.push_back(20);
	memset(buff, 42, 40);
	v.insert(v.end(), buff, buff + 40);
	v.insert(v.end(), buff, buff + 40);
	v.erase(v.begin(), v.begin()+40);
//	cout << v.size();
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


//	const char *args[3] = {"python3",
//						   "/Users/mehtel/coding/webserv/py.py",
//										  0};
//	execvp(args[0], (char* const*)args);
//	perror("");


	const std::time_t now = std::time(nullptr);
	char buf[64];
	if (strftime(buf, sizeof buf, "%a, %e %b %Y %H:%M:%S GMT\n", std::gmtime(&now))) {
//			std::cout << std::setw(40) << "    strftime %a %b %e %H:%M:%S %Y" << buf;
//		response += "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n";
		cout << "Mon, 27 Jul 2009 12:28:53 GMT\n";
		std::string a(buf);
		cout << a;
	} else
		cout << "err";

	int i = 12;
	for (; i < 30; i++);
	cout << "i=" << i;

	vector<string> filenames ;
	string result;
	filenames.push_back("youpi.bad_extension");
	find_file(filenames, "/Users/mehtel/coding/webserv", result);
	cout << "<<<<<<<<" << result << "\n";

	vector<int> arr;
	for (int i=0; i<100; i++) arr.push_back(i);

	for (int i=0; i<arr.size(); i++) {
		cout << "(" << i << ")";
//		cout << "(" << arr.size() << ")";
		if (arr[i] & 1) {
			arr.erase(arr.begin() + i);
			i--;
			continue;
		}
		cout << arr[i] << " ";
	}
	list<int> a;
	a.push_back(asd);

}
