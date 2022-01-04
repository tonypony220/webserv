#include <sys/poll.h>
#include "Session.hpp"
#include "Socket.hpp"
#include <vector>
#include <sys/socket.h>
//int add_listening_sockets(std::vector<Socket> sockets, struct pollfd fds,) {}

//* struct pollfd {
//* 	int   fd;         /* file descriptor */
//* 	short events;     /* requested events */
//* 	short revents;    /* returned events */
//* };
int timeout;
// [] [] [] [] []

int accept_connection(int listen_fd,
					  std::vector<Session> & sessions,
					  std::vector<pollfd> & fds) {
	struct pollfd new_fd;

	int new_socket_fd = accept(listen_fd, NULL, NULL);
	if (new_socket_fd == -1) {
		perror("accept error:");
	} else {
		new_fd.fd = new_socket_fd;
		new_fd.events = POLLIN;
		fds.push_back(new_fd);
		sessions.push_back(Session(new_socket_fd));
	}

}

int loop () {
	std::vector<struct pollfd> fds;
	//struct pollfd fds[200];
	std::vector<Session> sessions;
	std::vector<Socket>  sockets;
	struct pollfd new_fd;
	// int nfds;
//	pollfd ls;
//	ls.fd = 0;


	// adding sockets
//	int i = 0;
	for (std::vector<Socket>::iterator it = sockets.begin(); it != sockets.end(); it++) {
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
		for (int i = 0; i < sessions.size(); i++) {
			if (fds[i].revents == LISTENING_SESSION)
				continue;
			/// fds[i].revents != events => Error
			//this is expected if (fds[i].revents & POLLIN)
			if (sessions[i].getFd() == LISTENING_SESSION) {
				accept_connection(fds[i].fd, sessions, fds);
			}

//			fds[i].fd = it->getFd();
//			fds[i].events = POLLIN;
//			i++;
		}
	}
	return 0;
}

int main() {}
