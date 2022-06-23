#include <sys/poll.h>
#include "Session.hpp"
#include "SocketTCP.hpp"
#include "SmartPtr.hpp"
//#include "Server.hpp"
#include "ParseConf.hpp"
#include <vector>
#include <unordered_map>
#include <sys/socket.h>
#include <arpa/inet.h>

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

//int accept_connections(int listen_fd,
//					   Server & serv,
//					   std::vector<sptr<IOInterface> > & sessions,
//					   std::vector<pollfd> & fds) {
//	struct pollfd 	poll_fd;
//	//int 			new_socket_fd = 0;
//
////	while (new_socket_fd > -1)
////	{
////		std::cout << "adress of sssions: " << &sessions << std::endl;
//
//		struct sockaddr_in client_addr;
//		poll_fd.fd = accept(listen_fd, (struct sockaddr*)&client_addr, NULL);
//		struct in_addr ipAddr = client_addr.sin_addr;
//		char str[INET_ADDRSTRLEN];
//		inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );
//		log(&ipAddr);
//		log( std::string(str));
////	log(YELLOW"client ip="RESET, std::string(str));
////		poll_fd.fd = accept(listen_fd, NULL, NULL);
//		fcntl(poll_fd.fd, F_SETFL, O_NONBLOCK);
//		if (poll_fd.fd < 0 && errno != EWOULDBLOCK) {
//			log(RED"accept error: ", strerror(errno), RESET);
//		} else {
//			//poll_fd.fd = new_socket_fd;
//			poll_fd.events = POLLIN;
//			fds.push_back(poll_fd);
////			std::cout << "client ip=" << '\n';
////			log(GREEN"client ip="RESET);
//			log(YELLOW"accepted new connection fd=", poll_fd.fd, RESET);
//			sessions.push_back(sptr<IOInterface>( new tcpSession(poll_fd.fd, &serv)));
//		}
////		std::cout << "HERE2" << std::endl;
////	}
//	return 0;
//}

//std::vector<SocketTCP>  sockets;


int loop (Server & serv) {
	std::vector<struct pollfd>			fds;
	std::vector<sptr<IOInterface> > 	io_sessions;
	struct pollfd						poll_fd;

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
//				poll_fd.fd = accept(fds[i].fd, NULL, NULL);
				struct sockaddr_in client_addr;
				socklen_t len = sizeof(client_addr);

				poll_fd.fd = accept(fds[i].fd, (struct sockaddr*)&client_addr, &len);
				struct in_addr ipAddr = client_addr.sin_addr;
				char str[INET_ADDRSTRLEN];
				inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

				if (poll_fd.fd < 0 && errno != EWOULDBLOCK) {
					perror("accept error: ");
				} else {
					//poll_fd.fd = new_socket_fd;
					log(GREEN"accepted new connection, fd: ", poll_fd.fd, "ip: ", str);
					log(RESET);
					poll_fd.events = POLLIN | POLLOUT;
					fds.push_back(poll_fd);
					tcpSession * session = new tcpSession(poll_fd.fd, &serv);
					session->ip = std::string(str);
					io_sessions.push_back(sptr<IOInterface>(session));
				}
				//accept_connections(fds[i].fd, *io_sessions[i]->getServ(), io_sessions, fds);
				continue;
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
					log(BLUE"cgi created", poll_fd.fd);
				} // TODO handle error
				else
					log(RED"cgi failed"RESET, poll_fd.fd);
			}
//			else if ( ret == HANDLE_FILE ) {
//				log("file session creating...", poll_fd.fd);
//				File * cgi = io_sessions[i]->get_cgi_pipe();
//				if (cgi) {
//					poll_fd.events = POLLIN;
//					poll_fd.fd = cgi->getFd();
//					fds.push_back(poll_fd);
//					io_sessions.push_back(sptr<IOInterface>( cgi ));
//					log("cgi created", poll_fd.fd);
//				} // TODO handle error
//				log("cgi failed", poll_fd.fd);
//			}
			else if (ret != SUCCESS) {
				log(BLUE" closing connection fd=", poll_fd.fd, RESET);
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

	Server serv;
	ParserConfig parser("file");
	if (parser.parse_file() == EXIT_FAILURE
	|| serv.create(parser.configs) == EXIT_FAILURE )
		return EXIT_FAILURE;

//	p(parser.configs);
//	std::cout << GREEN" end "RESET << parser.configs.size() << " \n";

//	if ()
//		return 1;


//	Server_config config;
//	config.root = "/Users/mehtel/coding/webserv/";
//	config.add_port(2001);
//	config.add_port(2002);
//	config.error_pages_path="/Users/mehtel/coding/webserv/err_pages";
//
//	Location loc1;
//	loc1.route = "/";
//	loc1.enable_cgi("py");
//	loc1.enable_cgi("sh");
//	loc1.enable_cgi("cgi");
//	loc1.allow("GET");
//	loc1.allow("POST");
//	loc1.allow("DELETE");
//
//	serv.add_config(config);
	loop(serv);
	return EXIT_SUCCESS;
}
