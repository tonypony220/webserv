#include <sys/poll.h>
#include "tcpSession.hpp"
#include "SocketTCP.hpp"
#include "SmartPtr.hpp"
#include "Server.hpp"
#include <vector>
#include <unordered_map>
#include <sys/socket.h>

// pollfd orig structure
//
// struct pollfd {
// 	int   fd;         /* file descriptor */
// 	short events;     /* requested events */
// 	short revents;    /* returned events */
// };



int timeout;
int stop = 0;

//int close_connection(std::vector<sptr<IOInterface> > & sessions,
//					 std::vector<pollfd> & fds) {
//	return 0;
//}

int accept_connections(int listen_fd,
					   Server & serv,
					   std::vector<sptr<IOInterface> > & sessions,
					   std::vector<pollfd> & fds) {
	struct pollfd 	poll_fd;
	//int 			new_socket_fd = 0;

//	while (new_socket_fd > -1)
//	{
//		std::cout << "adress of sssions: " << &sessions << std::endl;
		poll_fd.fd = accept(listen_fd, NULL, NULL);
		if (poll_fd.fd < 0 && errno != EWOULDBLOCK) {
			perror("accept error: ");
		} else {
			//poll_fd.fd = new_socket_fd;
			poll_fd.events = POLLIN;
			fds.push_back(poll_fd);
			log("accepted new connection", poll_fd.fd);
			sessions.push_back(sptr<IOInterface>( new tcpSession(poll_fd.fd, &serv)));
		}
//		std::cout << "HERE2" << std::endl;
//	}
	return 0;
}

//std::vector<SocketTCP>  sockets;


int loop (Server & serv) {
	std::vector<struct pollfd>	fds;
	std::vector<sptr<IOInterface> > 	io_sessions;
	struct pollfd				poll_fd;

	int timeout = 1 * 1000; // 3 min (3 * 60 * 1000)

	// std::vector<SocketTCP> sockets = SocketTCP::getSockets();
	// adding sockets
	for (std::vector<SocketTCP>::iterator sockptr = serv.sockets.begin(); sockptr != serv.sockets.end(); sockptr++) {
		if (sockptr->openSocket())
			continue; // todo. exit on error
		poll_fd.fd = sockptr->getFd();
		poll_fd.events = POLLIN | POLLOUT;
		fds.push_back(poll_fd);
		io_sessions.push_back(sptr<IOInterface>(new tcpSession(LISTENING_SESSION, &serv)));
	}
//	std::cout << "adress of sssions: " << &tcpSessions << std::endl;
	while (1) {
		int rc = poll(&fds[0], fds.size(), timeout);
		if (rc < 0)
			perror("poll error");

//		std::cout << "Status: " << "fds:" << fds.size();
//		std::cout << " sessions:" << sessions.size();
//		std::cout << " poll=" << rc << std::endl

		for (int i = 0; i < io_sessions.size(); i++) {
			if ( ! (fds[i].revents & ( POLLIN | POLLOUT ) ) )// == 0 ??
				continue;
			//log("\tprocessing fd=", fds[i].fd);
			//log("\tprocessing fd=", fds[i].fd);
			io_sessions[i]->counter++;
			//std::cout << i << std::endl;
			/// fds[i].revents != events => Error
			/// this is expected if (fds[i].revents & POLLIN)
			if (io_sessions[i]->getFd() == LISTENING_SESSION && ( fds[i].revents & POLLIN )) {
				poll_fd.fd = accept(fds[i].fd, NULL, NULL);
				if (poll_fd.fd < 0 && errno != EWOULDBLOCK) {
					perror("accept error: ");
				} else {
					//poll_fd.fd = new_socket_fd;
					log("accepted new connection, fd: ", poll_fd.fd);
					poll_fd.events = POLLIN | POLLOUT;
					fds.push_back(poll_fd);
					io_sessions.push_back(sptr<IOInterface>( new tcpSession(poll_fd.fd, &serv)));
				}
				//accept_connections(fds[i].fd, *io_sessions[i]->getServ(), io_sessions, fds);
				continue;
			}
			int ret = io_sessions[i]->processEvent(fds[i].revents);
			if ( ret == HANDLE_CGI ) {
				log("cgi creating...", poll_fd.fd);
				CgiPipe * cgi = io_sessions[i]->get_cgi_pipe();
				if (cgi) {
					poll_fd.events = POLLIN;
					poll_fd.fd = cgi->getFd();
					fds.push_back(poll_fd);
					io_sessions.push_back(sptr<IOInterface>( cgi ));
					log("cgi created", poll_fd.fd);
				} // TODO handle error
				log("cgi failed", poll_fd.fd);
			}
			else if (ret != SUCCESS) {
//				close_connection(sessions, fds);
				close(fds[i].fd);
				// https://stackoverflow.com/questions/9927163/erase-element-in-vector-while-iterating-the-same-vector
				io_sessions.erase(io_sessions.begin() + i);
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
	std::unordered_map<int, int> d;
	std::vector<int> ports; 
	ports.push_back(2002);
	ports.push_back(2001);
	Server serv(ports, "~/coding/webserv");
	/* sockets.push_back(SocketTCP(2002)); */
	/* sockets.push_back(SocketTCP(2001)); */

//	sockets.push_back(sock);
	loop(serv);
}
