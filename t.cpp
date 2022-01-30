#include "HttpSession.hpp"
#include <vector>

int main () {
	std::vector<HttpSession> v;
	HttpSession s(2);
//	Session ss(3);

	v.push_back(s);
	v.push_back(HttpSession(33));

	for(std::vector<HttpSession>::iterator it = v.begin(); it != v.end(); ++it) {
		std::cout << *it << std::endl;
	}
}