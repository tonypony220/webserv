#include <sys/select.h>
#include "HttpSession.hpp"
#include <vector>

int ls; // listening socket
std::vector<HttpSession> sessions;

int main () {
	while (1) {
		//int fd, res;
		int max_d = ls;
		fd_set read_fds;
		fd_set write_fds;
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		FD_SET(ls, &read_fds);

		std::vector<HttpSession>::iterator session;
		for (session = sessions.begin(); session < sessions.end(); session++) {
			FD_SET(session->getFd(), &read_fds);
			if (session->toBeWrite())
				FD_SET(session->getFd(), &write_fds);
			if (session->getFd() > max_d)
				max_d = session->getFd();
		}

		struct timeval timeout;
		timeout.tv_sec = 5;

		int res = select(max_d + 1, &read_fds, &write_fds, NULL, &timeout);

		if (res == -1) {
			if (errno == EINTR) {
				/// signal handling
			} else {
				/// select error
			}
			continue;
		}
		if (res == 0) // time out
			continue;
		if (FD_ISSET(ls, &read_fds))
			accept_connection(ls);

		for (session = sessions.begin(); session < sessions.end(); session++) {
			if (FD_ISSET(session->getFd(), &read_fds))
				///	reading
				reading(*session);
			if (FD_ISSET(session->getFd(), &write_fds))
				///	writing
				writing(*session);

		}
	}
	return 0;
}