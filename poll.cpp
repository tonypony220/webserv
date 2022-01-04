#include <sys/poll.h>
#include "Session.hpp"
#include "Socket.hpp"
//int add_listening_sockets(std::vector<Socket> sockets, struct pollfd fds,) {}

//* struct pollfd {
//* 	int   fd;         /* file descriptor */
//* 	short events;     /* requested events */
//* 	short revents;    /* returned events */
//* };
int timeout;
// [] [] [] [] []

int loop () {
	std::vector <pollfd> fds;
	//struct pollfd fds[200];
	std::vector<Session> sessions;
	std::vector<Socket>  sockets;
	// int nfds;
//	pollfd ls;
//	ls.fd = 0;


	// adding sockets
//	int i = 0;
	for (std::vector<Socket>::iterator it = sockets.begin(); it != sockets.end(); it++) {
		struct pollfd new_fd;
		new_fd.fd = it->getFd();
		new_fd.events = POLLIN;
		fds.push_back(new_fd);
		sessions.push_back(Session(0));
	//	i++;
	//	nfds++;
	}
	timeout = (3 * 60 * 1000); // 3 min

	while (1) {
		int rc = poll(&fds[0], fds.size(), timeout);
		if (rc < 0)
			perror("poll error");
		for (int i = 0; i < sessions.size(), i++) {
			if (fds[i].revents == LISTENING_SESSION)
				continue;
			/// fds[i].revents != events => Error
			if (fds[i].revents & POLLIN)
				sessions[i].fd != 0 || create_session();
//			fds[i].fd = it->getFd();
//			fds[i].events = POLLIN;
//			i++;
		}

		for (int i = 0; i < nfds; i++)
		{

		}
	}
}
