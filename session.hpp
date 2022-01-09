#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "Request.hpp"
#include "Response.hpp"
#define LISTENING_SESSION 0 // zero stub for vector of FDs
#define BUFF_SIZE 200
#define DONE 0
//#define MORE 1

class Session {
	private:
		int	 		 fd; // 0 if listening fd
		HttpRequest  request;
		HttpResponse response;

		bool 		 verbose;

	public:
		Session( int fd ) : fd(fd), request(fd), response(fd) {
			verbose = true;
			verbose && fd && std::cout << "Session created, fd: " << fd << std::endl;
//			verbose && !fd && std::cout << "Listening.." << std::endl;
		}

		Session( const Session & copy ) : fd(copy.getFd()), request(copy.getRequest()), response(copy.getResponse()) {} //, buffer(copy.getBuff()) {}

		~Session( void ) {
//			verbose && std::cout << "Session destructed, fd: " << fd << std::endl;
		}

		Session & operator=( const Session & other ) {
			fd = other.getFd();
			request = other.getRequest();
			response = other.getResponse();
			return *this;
		}

		int 		 getFd( void )      const { return fd; }
		HttpRequest  getRequest( void )  const { return request; }
		HttpResponse getResponse( void ) const { return response; }
//		bool 		 somethingToWrite( void ) const { return !response.empty(); }
		int 		 process_event( short event ) {
			if (event == POLLIN) {
				int rc = request.readSocket();
				std::cout << "buffer: " << request.getBuffer() << "$" << std::endl;
				if (rc == DONE)
//					std::cout << "buffer: " << request.getBuffer() << "$" << std::endl;
				return rc;
			}
			else if (event == POLLOUT)
				return response.writeSocket();
//				return writeSocket();
			return 0;
		}
};

std::ostream & operator<<( std::ostream & o, Session & s ) {
	o << "fd: " << s.getFd();
//	o << " buff: " << s.getRequest() << std::endl;
	return ( o );
}



