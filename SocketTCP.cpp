#include "SocketTCP.hpp"

SocketTCP::SocketTCP(int port) : port(port) {
//			std::chrono::time_point<std::chrono::high_resolution_clock> t_start;
		}
SocketTCP::~SocketTCP() {}
		/* it's should not be copyable but it's better than use pointers */
SocketTCP::SocketTCP( const SocketTCP & copy ) { *this = copy; }

SocketTCP & SocketTCP::operator=( SocketTCP const & other ) {
			if ( &other != this ) {
				port    = other.port ;
				fd	    = other.fd;
				buff  	= other.buff;
			}
			return *this;
		}
		// todo correct error handling
		int SocketTCP::openSocket() {
		    /* Create an AF_INET6 stream socket to receive incoming      */
		    /* connections on                                           */
			int rc;
			fd = socket(AF_INET, SOCK_STREAM, 0);
			if ( fd < 0 ) {
				buff = std::string("socket error: ") + std::string(std::strerror(errno));
				std::cerr << buff;
				return -1;
			}
			fcntl(fd, F_SETFL, O_NONBLOCK);
			int opt = 1;
			/* Allow socket descriptor to be reuseable                   */
			rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  			if (rc < 0) {
  			  perror("setsockopt() failed");
  			  close(fd);
  			  //exit(-1);
			}
  			

		    /* Bind the socket                                           */
			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr)),
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin_port = htons(port);

			rc = bind(fd, (struct sockaddr*) &addr, sizeof(addr));
			if( rc < 0 ) {
				buff = std::string("bind error: ")
					 + std::string(std::strerror(errno));
				std::cerr << RED << buff << RESET;
				close(fd);
				fd = -1;
				return -1;
			}
			/* Set the listen back log                                   */
			rc = listen(fd, LISTEN_QLEN);
		    if (rc < 0)	{
			  perror("listen() failed");
			  close(fd);
			  return -1;
//			  exit(-1);
			}

			log("Socket listening, port: ", port);
			return 0;
		}

		int SocketTCP::closeSocket() {
			log("Socket closed, port: " ,port);
			return close(fd); // 0:ok -1:err
		}


		int 	 	SocketTCP::getFd()   const { return fd; }
		std::string SocketTCP::getBuff() const { return buff; }
		int		    SocketTCP::getPort() const { return port; }

std::ostream & operator<<( std::ostream & o, SocketTCP & s ) {
	o << "SocketTCP: fd=" << s.getFd() << " port=" << s.getPort();
//	o << " buff: " << s.getBuff() << std::endl;
	return ( o );
}
