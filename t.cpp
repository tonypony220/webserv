#include "session.hpp"
#include <vector>

int main () {
	std::vector<Session> v;
	Session s(2);
//	Session ss(3);

	v.push_back(s);
	v.push_back(Session(33));

	for(std::vector<Session>::iterator it = v.begin(); it != v.end(); ++it) {
		std::cout << *it << std::endl;
	}
}