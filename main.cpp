#include "SocketTCP.hpp"

//template <class T>
//void pr(T & a) {
//	std::cout << a << std::endl;
//}

#include <sys/poll.h>
#include <algorithm>
#include <vector>
#include "iter.hpp"

void func_on_fd(const pollfd & fd) {
	std::cout << "FUNC: " << fd.fd << std::endl;
}

int main () {
	SocketTCP s;
	SocketTCP d;
	std::cout << s.getFd() << std::endl;
	std::cout << d.getFd() << std::endl;
//	pr("hello");
//	pr(s.getBuff());
	if (s.getFd() < 0)
		std::cout << s.getBuff() << std::endl;
	if (d.getFd() < 0)
		std::cout << d.getBuff() << std::endl;
	std::vector<pollfd> fds;
//	for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
//	{}
	pollfd ls = *(new pollfd);
	pollfd ls2;
	ls.fd = 4;
	fds.push_back(ls);
	ls.fd = 2;
	fds.push_back(ls);
	ls.fd = 3;
	fds.push_back(ls);
	std::cout << "FD: " << (static_cast<struct pollfd*>(&fds[0]))->fd << std::endl; 	  ///(struct pollfd)
	std::cout << "FD: " << (static_cast<struct pollfd*>(&fds[0]) + 1)->fd << std::endl; ///(struct pollfd)
	std::cout << "FD: " << (static_cast<struct pollfd*>(&fds[0]) + 2)->fd << std::endl; ///(struct pollfd)
	std::cout << "size: " << fds.size() << std::endl;
	std::cout << "FD: " << fds[3].fd << std::endl;
	std::cout << "size: " << fds.size() << std::endl;

	std::for_each(fds.begin(), fds.end(), func_on_fd);
	ffor(fds, func_on_fd);
//	std::string h= "HELLO";
//	write(0, h.c_str(), h.size());
	char xd[] = "hello56789";
//	memset(xd + 7, 1, sizeof(xd) - 7);
	memset(xd + 9, 0, sizeof(xd) - 9);
	std::cout << std::string(xd) << std::endl;
}
