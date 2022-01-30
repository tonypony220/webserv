#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "Request.hpp"
#include "Response.hpp"
#define LISTENING_SESSION 0 // zero stub for vector of FDs
#define BUFF_SIZE 3
#define DONE 0
//#define MORE 1


class HttpSession {
  private:

	int	 		 			  fd; // 0 if listening fd
	std::vector<HttpParser>   requests;
	std::vector<HttpResponse> responses;
	unsigned int 			  current_request;
	unsigned int 			  current_response;

//	typedef std::vector<HttpParser>::iterator sesstor;
//	sesstor 				current_request;
//	sesstor 				current_response;
	std::string 			buffer;

//	sessions[current_request]->write();

//		HttpRequest  request;
//		HttpResponse response;

	bool 		 verbose;

  public:
	HttpSession(int fd ) : fd(fd) {
		requests.push_back(HttpParser());
		current_request  = 0;
		current_response = 0;
//		current_request = messages.begin();
//		current_response = messages.begin();

		verbose = true;
		verbose && fd && std::cout << "HttpSession created, fd: " << fd << std::endl;
//			verbose && !fd && std::cout << "Listening.." << std::endl;
	}

//todo	Session( const Session & copy ) : fd(copy.getFd()), request(copy.getRequest()), response(copy.getResponse()) {} //, buffer(copy.getBuff()) {}
	~HttpSession(void ) { verbose && std::cout << "Session destructed, fd: " << fd << std::endl;}

	HttpSession( const HttpSession & copy ) { *this = copy; }

//	HttpSession & operator=( const HttpSession & other ) {
//		messages = other.messages;
//		current_request = messages.begin();
//		current_response = messages.begin();
//		buffer = other.buffer;
//		fd = other.getFd();
//		return *this;
//	}

	int 		 getFd( void )      const { return fd; }
//	HttpRequest  getRequest( void )  const { return request; }
//	HttpResponse getResponse( void ) const { return response; }
//	bool 		 somethingToWrite( void ) const { return !response.empty(); }

	int 		 processEvent( short event ) {
		if (event & POLLIN) {
			verbose && std::cout << "\tsession reading..." << fd << std::endl;
			if (!readSocket())
				return 0;

//			std::string buff = request.getBuffer();
//				replace(buff, "\n", "LF\n");
//				replace(buff, "\r", "CR\r");

//			replace(buff, "\r\n", " CRLF ");
//			std::cout << "bukfer: " << buff << "$" << std::endl;

//				if (rc == DONE)
//					std::cout << "buffer: " << request.getBuffer() << "$" << std::endl;
//			return rc;
		}
		if (!responses.empty() && (event & POLLOUT) && responses[current_response].isResponseDone()) { // TCP buffer have space to write to
			verbose && std::cout << "\tsession writing..." << fd << std::endl;
			return writeSocket();
		}
//				return writeSocket();
		return 1;
	}

	int  		  writeSocket() {
		int rc = ::write(fd, responses[current_response].getBuffer().c_str(), responses[current_response].getBuffer().size());
		if (rc <= 0) {
			if (rc < 0)
				std::cerr << "write error: " << strerror(errno);
			return 0;
		}
		buffer.erase(0, rc);
//		buffer.clear();
		return 1;
	}

	int 		   readSocket() {
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		int rc = ::read(fd, buff, BUFF_SIZE - 1);
		if (rc <= 0) {
			if (rc < 0)
				std::cerr << "read error: " << strerror(errno);
			return 0;
		}
		buffer.append( buff );
		requests[current_request].parseInput(buffer);
		if (requests[current_request].isRequestDone()) {
			HttpResponse resp = HttpResponse(requests[current_request]);
			std::cout << resp << std::endl;
			responses.push_back(HttpResponse(requests[current_request]));
		}
		if (requests[current_request].isRequestDone() && !buffer.empty())
		{
			requests.push_back(HttpParser());
			current_request++;
		}
		return 1;
	}

//	HttpParser & getSessionToRead() {
//		for (sesstor it = messages.begin(); it != messages.end(); ++it) {
//			if (!it->isRequestDone())
//				return *it;
//		}
//	}
};


std::ostream & operator<<(std::ostream & o, HttpSession & s ) {
	o << "fd: " << s.getFd();
//	o << " buff: " << s.getRequest() << std::endl;
	return ( o );
}



