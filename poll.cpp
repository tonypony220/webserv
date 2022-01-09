#include <sys/poll.h>
#include "Session.hpp"
#include "SocketTCP.hpp"
#include <vector>
#include <sys/socket.h>

// pollfd orig structure
struct pollfddddd {
	int   fd;         /* file descriptor */
	short events;     /* requested events */
	short revents;    /* returned events */
};


int timeout;
int stop = 0;

int close_connection(std::vector<Session> & sessions,
					 std::vector<pollfd> & fds) {
	return 0;
}

int accept_connections(int listen_fd,
					   std::vector<Session> & sessions,
					   std::vector<pollfd> & fds) {
	struct pollfd new_fd;
	int new_socket_fd = 0;

//	while (new_socket_fd > -1)
//	{
//		std::cout << "HERE1" << std::endl;
		new_socket_fd = accept(listen_fd, NULL, NULL);
		if (new_socket_fd == -1 && errno != EWOULDBLOCK) {
			perror("accept error: ");
		} else {
			new_fd.fd = new_socket_fd;
			new_fd.events = POLLIN;
			fds.push_back(new_fd);
			sessions.push_back(Session(new_socket_fd));
		}
//		std::cout << "HERE2" << std::endl;
//	}
	return 0;
}

//std::vector<SocketTCP>  sockets;


int loop () {
	std::vector<struct pollfd> fds;
	std::vector<Session> sessions;
	struct pollfd new_fd;
	int timeout = 1 * 1000; // 3 min (3 * 60 * 1000)

	std::vector<SocketTCP> sockets = SocketTCP::getSockets();
	// adding sockets
	for (std::vector<SocketTCP>::iterator it = sockets.begin(); it != sockets.end(); it++) {
		if (it->openSocket())
			continue;
		new_fd.fd = it->getFd();
		new_fd.events = POLLIN;
		fds.push_back(new_fd);
		sessions.push_back(Session(0));
	}
	for (;;) {
		int rc = poll(&fds[0], fds.size(), timeout);
		if (rc < 0)
			perror("poll error");

		std::cout << "Status: " << "fds:" << fds.size();
		std::cout << " sessions:" << sessions.size();
		std::cout << " poll=" << rc << std::endl;

		for (int i = 0; i < sessions.size(); i++) {
//			std::cout << i << std::endl;
			if (fds[i].revents == 0)
				continue;
			/// fds[i].revents != events => Error
			/// this is expected if (fds[i].revents & POLLIN)
			if (sessions[i].getFd() == LISTENING_SESSION) {
				accept_connections(fds[i].fd, sessions, fds);
				continue;
			}
			if (sessions[i].process_event(fds[i].revents) == 0) {
//				close_connection(sessions, fds);
				close(fds[i].fd);
				// https://stackoverflow.com/questions/9927163/erase-element-in-vector-while-iterating-the-same-vector
				sessions.erase(sessions.begin() + i);
				fds.erase(fds.begin() + i);
				i--; // keeps index in same place;
			}
		}
		if (stop)
			break;
	}
	return 0;
}

int main() {
	SocketTCP sock(2002);
	SocketTCP socx(2001);


//	sockets.push_back(sock);
	loop();
}
