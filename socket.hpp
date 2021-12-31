#pragma once
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#define LISTEN_QLEN 32

class Socket {
	private:
		int 		port;	
		int 		fd;
		std::string buff; 

		void operator=(Socket const &) {}
		Socket(const Socket & copy) {}
	public:

		Socket( int port = 80 ) : port(port) {
			/* std::cout << "buff: " << buff ; */
    		fd = socket(AF_INET, SOCK_STREAM, 0);
			if (fd == -1) {
				buff = std::string("socket error: ") + std::string(std::strerror(errno));
				return ;
			}
    		int opt = 1;
    		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    		struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin_port = htons(port);

			if(-1 == bind(fd, (struct sockaddr*) &addr, sizeof(addr))) {
				buff = std::string("bind error: ") + std::string(std::strerror(errno));
				close(fd);
				fd = -1;
				return ;
			}
			listen(fd, LISTEN_QLEN);
    	}

		~Socket() {
			close(fd);
			std::cout << port << "- port, socket closed" << std::endl;
		}
		int 		getFd()   const { return fd; }
		std::string getBuff() const { return buff; }
		int		    getPort() const { return port; }



};
