#include <sys/poll.h>
#include <vector>
#include <list>
#include <unordered_map>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "Session.hpp"
#include "SocketTCP.hpp"
#include "SmartPtr.hpp"
#include "Server.hpp"
#include "ParseConf.hpp"
//
//	POLL example
//	https://www.ibm.com/docs/en/i/7.4?topic=designs-using-poll-instead-select
//
//	pollfd orig structure
//
//	struct pollfd {
//		int   fd;         /* file descriptor */
//		short events;     /* requested events */
//		short revents;    /* returned events */
//	};


bool stop_server = false;

void sigpipe_handler(int s) { // this not expected
	printf(RED"Caught SIGPIPE!\n"RESET);
	stop_server = true;
}
void sigint_handler(int s) {
	printf(GREEN"Shutting down...\n"RESET);
	stop_server = true;
}

int loop (Server & serv) {
	std::vector<struct pollfd>			fds;
	std::vector<sptr<IOInterface> > 	io_sessions;
	struct pollfd						poll_fd;

	int timeout = 1 * 1000; // 3 min (3 * 60 * 1000)
	signal(SIGPIPE, sigpipe_handler);
	signal(SIGINT,	sigint_handler);

	io_sessions.reserve(100);

	// adding sockets
	for (std::vector<SocketTCP>::iterator sockptr = serv.sockets.begin();
		sockptr != serv.sockets.end();
		sockptr++) {
		if (sockptr->openSocket())
			return EXIT_FAILURE;  // not protected
		poll_fd.fd = sockptr->getFd();
		fcntl(poll_fd.fd, F_SETFL, O_NONBLOCK);
		poll_fd.events = POLLIN | POLLOUT;
		fds.push_back(poll_fd);
		io_sessions.push_back(sptr<IOInterface>(new tcpSession(LISTENING_SESSION, &serv)));
	}
//	std::cout << "adress of sssions: " << &tcpSessions << std::endl;

	while (1) {
		std::cout << "\r\t\t\t\t\t\t\t\t\t\t\t sessions:" << io_sessions.size();
		int rc = poll(&fds[0], fds.size(), timeout);
		if (rc < 0)
			perror("poll error");
//		std::cout << "Status: " << "fds:" << fds.size();
//		std::cout << " poll=" << rc << std::endl
//		for (int i = 0; i < io_sessions.size(); i++) {
//			std::cout << io_sessions[i] << " ";
//		}
//		std::cout.flush();
		for (int i = 0; i < io_sessions.size(); i++) {
//			if ( ! (fds[i].revents & ( POLLIN | POLLOUT ) ) )// == 0 ??
//				continue;
			//log("\tprocessing fd=", fds[i].fd);
			//log("\tprocessing fd=", fds[i].fd);
			io_sessions[i]->counter++;
			//std::cout << i << std::endl;
			// fds[i].revents != events => Error
			// this is expected if (fds[i].revents & POLLIN)
			if (io_sessions[i]->getFd() == LISTENING_SESSION ) {
				if ( !(fds[i].revents & POLLIN ))
					continue;
//				poll_fd.fd = accept(fds[i].fd, NULL, NULL);
				struct sockaddr_in client_addr;
				socklen_t len = sizeof(client_addr);
				poll_fd.fd = 0;
				/* potential bottleneck */
				while (poll_fd.fd != -1) {
					poll_fd.fd = accept(fds[i].fd, (struct sockaddr*)&client_addr, &len);
					struct in_addr ipAddr = client_addr.sin_addr;
					char str[INET_ADDRSTRLEN];
					inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

					if (poll_fd.fd < 0){ // && errno != EWOULDBLOCK) {
						if (errno != EWOULDBLOCK )
							perror("accept error: ");
					} else {
						fcntl(poll_fd.fd, F_SETFL, O_NONBLOCK);
						//poll_fd.fd = new_socket_fd;
						log(GREEN"accepted new connection, fd: ", poll_fd.fd, "ip: ", str);
						log(RESET);
						poll_fd.events = POLLIN | POLLOUT;
						fds.push_back(poll_fd);
						tcpSession * session = new tcpSession(poll_fd.fd, &serv);
						session->ip = std::string(str);
						sptr<IOInterface> p(session);
//					io_sessions.push_back(sptr<IOInterface>(session));
//					std::cout << PURPLE"ptr IOinterface createed: " << p << "\n"RESET;
						io_sessions.push_back(p);
					}
				}

				//accept_connections(fds[i].fd, *io_sessions[i]->getServ(), io_sessions, fds);
				break;
			}
			int ret = io_sessions[i]->processEvent(fds[i].revents);
//			log(BLUE"\rprocessed event=", ret, RESET);
//			std::cout << BLUE"\rprocessed event fd=" << fds[i].fd << "ret=" << ret << " POLLIN=" << (fds[i].revents & POLLIN) << RESET;
			if ( ret == ADD_IFCE ) {
				log(BLUE"cgi session creating..."RESET, poll_fd.fd);
				//CgiPipe * cgi = io_sessions[i]->get_cgi_pipe();
				IOInterface * io = io_sessions[i]->get_interface();
				if (io) {
					poll_fd.events = POLLIN;
					poll_fd.fd = io->getFd();
					fds.push_back(poll_fd);
					io_sessions.push_back(sptr<IOInterface>( io ));
					log(BLUE"new interface created", poll_fd.fd);
				} // TODO handle error
				else
					log(RED"new interface failed"RESET, poll_fd.fd);
				break;
			}
			else if (ret != SUCCESS) {
//				std::cout << PURPLE"ptr IOinterface deleting: " << *(io_sessions.begin() + i) << "\n"RESET;
				log(BLUE" closing connection fd=", fds[i].fd, RESET);
//				close_connection(sessions, fds);
				int closed = close(fds[i].fd);
				log(BLUE" closed=", !closed, RESET);
//				std::cout.flush();
				// https://stackoverflow.com/questions/9927163/erase-element-in-vector-while-iterating-the-same-vector
				std::vector<sptr<IOInterface> >::iterator x;
//				std::cout << "iosession size =" << io_sessions.size() << "\n";
				x = io_sessions.erase(io_sessions.begin() + i);
//				std::cout << "iosession size =" << io_sessions.size() << "\n";
//				std::cout << "ptr: " << (x == io_sessions.end() ) << " ";
				fds.erase(fds.begin() + i);
				i--; // keeps index in same place;
			}
		}
		if (stop_server)
			break;
	}

	for (std::vector<SocketTCP>::iterator sockptr = serv.sockets.begin();
		 sockptr != serv.sockets.end();
		 sockptr++)
	{
		sockptr->closeSocket();
	}
	return 0;
}

int main() {
	Server serv;
	ParserConfig parser("conf");
	if (parser.parse_file() == EXIT_FAILURE
	|| serv.create(parser.configs) == EXIT_FAILURE )
		return EXIT_FAILURE;
	if (loop(serv))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
