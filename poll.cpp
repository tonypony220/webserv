#include <sys/poll.h>
#include "Session.hpp"
//int add_listening_sockets(std::vector<Socket> sockets, struct pollfd fds,) {}

//* struct pollfd {
//* 	int   fd;         /* file descriptor */
//* 	short events;     /* requested events */
//* 	short revents;    /* returned events */
//* };
int timeout;

int loop () {
//	std::vector <pollfd> l;
	struct pollfd fds[200];
	std::vector<Session> sessions;
	std::vector<Socket>  sockets;
	int nfds;
//	pollfd ls;
//	ls.fd = 0;

	// adding sockets
	int i = 0;
	for (std::vector<Socket>::iterator it = sockets.begin(); it != sockets.end(); it++) {
		fds[i].fd = it->getFd();
		fds[i].events = POLLIN;
		i++;
		nfds++;
	}
	timeout = (3 * 60 * 1000); // 3 min

	while (1) {
		for (int i = 0; i < nfds; i++) {

		}
	}
}
