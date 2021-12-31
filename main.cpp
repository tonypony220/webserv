#include "socket.hpp"

//template <class T>
//void pr(T & a) {
//	std::cout << a << std::endl;
//}

#include <sys/poll.h>
#include <vector>

int main () {
	Socket s;
	Socket d;
	std::cout << s.getFd() << std::endl;
	std::cout << d.getFd() << std::endl;
//	pr("hello");
//	pr(s.getBuff());
	if (s.getFd() < 0)
		std::cout << s.getBuff() << std::endl;
	if (d.getFd() < 0)
		std::cout << d.getBuff() << std::endl;
	std::vector<pollfd> l;
	pollfd ls;
	ls.fd = 2;
	l.push_back(ls);
	std::cout << "size: " << l.size() << std::endl;
	std::cout << "FD: " << l[3].fd << std::endl;
	std::cout << "size: " << l.size() << std::endl;
}
