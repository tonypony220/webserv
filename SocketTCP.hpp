#pragma once
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <netinet/in.h>
#define LISTEN_QLEN 32

// member vector of objects same class
class SocketTCP;
static std::vector<SocketTCP> ss;

class SocketTCP {
	private:
		int 		port;	
		int 		fd;
		std::string buff;

		bool verbose;

	public:

		int openSocket() {
			/* std::cout << "buff: " << buff ; */
			fd = socket(AF_INET, SOCK_STREAM, 0);
			if (fd == -1) {
				buff = std::string("socket error: ") + std::string(std::strerror(errno));
				return -1;
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
				return -1;
			}
			listen(fd, LISTEN_QLEN);
			verbose && std::cout << "Socket listening, port: " << port << std::endl;
			return 0;
		}

		int closeSocket() {
			verbose && std::cout << "Socket closed, port: " << port << std::endl;
			return close(fd); // 0:ok -1:err
		}

		SocketTCP(int port = 80 ) : port(port) {
			// creating socket (fd) in ctor makes difficult to move obj in vectors and so on;
			verbose = true;
			ss.push_back(*this);
    	}

		SocketTCP( const SocketTCP & copy ) { *this = copy; }

		~SocketTCP() {
		}

		SocketTCP & operator=( SocketTCP const & other ) {
			port    = other.port ;
			fd	    = other.fd;
			buff  	= other.buff;
			verbose = other.verbose;
			return *this;
		}

		int 		getFd()   const { return fd; }
		std::string getBuff() const { return buff; }
		int		    getPort() const { return port; }

		static const std::vector<SocketTCP> & getSockets() { return ss; }

};

std::ostream & operator<<( std::ostream & o, SocketTCP & s ) {
	o << "Socket: fd=" << s.getFd() << " port=" << s.getPort();
//	o << " buff: " << s.getBuff() << std::endl;
	return ( o );
}
