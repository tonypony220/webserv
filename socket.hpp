#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

class Socket {
	private:
		int port;	
		int fd;
		std::string buff; 

		void operator=(Socket const &) {}
		Socket(const Socket & copy) {}
	public:

		Socket( int port = 80 ) : port(port) {
    		fd = socket(AF_INET, SOCK_STREAM, 0);
			if (fd == -1) {
				buff = "socket error";
				return ;
			}
    		int opt = 1;
    		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    		struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin_port = htons(port);

			if(-1 == bind(fd, (struct sockaddr*) &addr, sizeof(addr)))
				perror("bind");
    }

    listen(sock, LISTEN_QLEN);
		}

		~Socket() {}



};
