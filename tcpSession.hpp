#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "Request.hpp"
#include "Utils.hpp"
#include "Response.hpp"
#include "Server.hpp"
#define LISTENING_SESSION 0 // zero stub for vector of FDs
//#define BUFF_SIZE 3
//#define MORE 1

class IOInterface {
protected:
	int	 		 	fd; // 0 if listening fd
public: 
	IOInterface(int  fd_, Server * serv) 
		: fd(fd_), server_ptr(serv) 
	{
		log("IOInterface created fd: ", fd);
	}
	virtual			~IOInterface() {}
	int 		 	getFd( void ) const { return fd; }
	Server * 		getServ( void ) const { return server_ptr; }
	virtual int		processEvent( short event ) {};
	virtual CgiPipe * get_cgi_pipe() { return NULL; }

	Server *				  server_ptr;
};

class CgiPipe : public IOInterface {
public:
	std::string		buffer;
	HttpResponse *	response_ptr;
	/* int fd_in; */
	/* int fd_out; */

	CgiPipe(int fd, HttpResponse * resp) 
		: IOInterface(fd, NULL), response_ptr(resp) {} // NULL stub 

	virtual int		processEvent( short event ) {
		int ret;
		if (event & POLLIN) {
			//log("\tsession reading...fd=",fd);
			ret = readPipe();
			if (ret == ERROR)
				return ERROR;
		}
	//	if ( !response_ptr->buffer.empty ) { //event & POLLOUT && 
	//		ret = writePipe();
	//		if (ret == ERROR)
	//			return ERROR;
		//	log("event POLLUT");
		//	if ( ret == END && responses.back().sent() ) // TODO next response
		//		return END;
		//	if (responses[current_response].ready()) { // TCP buffer have space to write to
		//		log("\tsession writing...", fd);
		//		writeSocket();
		//	}
//		}
		return SUCCESS;
	}
	int 		   readPipe() {
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		int rc = read(fd, buff, BUFF_SIZE - 1);
		if (rc == 0) 
			return END;
		if (rc < 0) {
			std::cerr << "read error: " << strerror(errno);
			return ERROR;
		}
		buffer.append( buff );
	}

//	int  		  writePipe() {
//		//log("write to socket:", buff); //fd &&
//		std::string buff = response_ptr->buffer;
//		int 		rc = ::write(fd_out, buff.c_str(), buff.size());
//		if (rc < 0) {
//			std::cerr << "write error: " << strerror(errno);
//			return ERROR;
//		} //if (rc == 0) { ????????????
//		buffer.erase(0, rc);
//		return SUCCESS;
//	}
};

class tcpSession : public IOInterface {
  private:
	//int	 		 			  fd; // 0 if listening fd
	std::vector<HttpParser>   requests;
	std::vector<HttpResponse> responses;
	unsigned int 			  current_request;
	unsigned int 			  current_response;
	std::string 			  buffer;

  public:

	tcpSession(int fd, Server * serv)
		: IOInterface(fd, serv),
		current_response(0),
		current_request(0) {
		add_request();
		log("tcpSession created, fd: ", fd); //fd &&
	}
	~tcpSession() { 
		log("Session destructed, fd: ", fd);
	}
//	tcpSession( const tcpSession & copy )
//		: fd(copy.fd), server_ptr(copy.server_ptr) { *this = copy; }
//
//	tcpSession & operator=( tcpSession const & other ) {
//		fd = other.fd;
//		requests = other.requests;
//		responses = other.responses;
//		server_ptr = other.server_ptr;
//		current_response = other.current_response;
//		current_request = other.current_request;
//		buffer = other.buffer;
//	}
	
	//{ *this = copy; }
//	HttpRequest  getRequest( void )  const { return request; }
//	HttpResponse getResponse( void ) const { return response; }
//	bool 		 somethingToWrite( void ) const { return !response.empty(); }
	void 		 add_request() {
		requests.push_back(HttpParser(server_ptr));
	}
	CgiPipe * 	 get_cgi_pipe() { 
		int fd = responses[current_response].cgi(); 
		if (fd == ERROR)
			return NULL;
		return new CgiPipe(fd, &responses[current_response]);
	}

	int 		 processEvent( short event ) {
		int ret;
		if (event & POLLIN) {
			log("\tsession reading...fd=",fd);
			ret = readSocket();
			if (ret == ERROR)
				return ERROR;
		}
		if ( !responses.empty() ) { 
			responses[current_response].make_response();
			if ( responses[current_response].is_cgi() )
				return HANDLE_CGI;
		//	log("event POLLUT");
			if ( responses[current_response].ready() && (event & POLLOUT) ) { // TCP buffer have space to write to
				log("\tsession writing...", fd);
				ret = writeSocket();
				if ( ret == END && responses[current_response].sent() ) // TODO next response
					return END;
			}
		}
		return SUCCESS;
	}
//			std::string buff = request.getBuffer();
//				replace(buff, "\n", "LF\n");
//				replace(buff, "\r", "CR\r");

//			replace(buff, "\r\n", " CRLF ");
//			std::cout << "bukfer: " << buff << "$" << std::endl;

//				if (rc == DONE)
//					std::cout << "buffer: " << request.getBuffer() << "$" << std::endl;
//			return rc;

	int  		  writeSocket() { 
		//TODO maybe not sent all response for once cause
		// e
		std::string buff = responses[current_response].get_response();
		log("write to socket:", PURPLE, buff, RESET); //fd &&
		int 		rc = ::write(fd, buff.c_str(), buff.size());
		if (rc < 0) {
			std::cerr << "write error: " << strerror(errno);
			return ERROR;
		} //if (rc == 0) { ????????????
		buffer.erase(0, rc);
		//responses[current_response].
		return END;
	}

	int 		   readSocket() {
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		int rc = read(fd, buff, BUFF_SIZE - 1);
		if (rc == 0) 
			return END;
		if (rc < 0) {
			std::cerr << "read error: " << strerror(errno);
			return ERROR;
		}
		buffer.append( buff );
		requests[current_request].parseInput( buffer );

		if (requests[current_request].isComplete()) {
			HttpResponse resp = HttpResponse(requests[current_request]);
			std::cout << resp << std::endl;
			responses.push_back(resp);
		}
		if (requests[current_request].isComplete() && !buffer.empty()) {
			add_request();
			current_request++;
		}
		return SUCCESS;
	}

//	HttpParser & getSessionToRead() {
//		for (sesstor it = messages.begin(); it != messages.end(); ++it) {
//			if (!it->isComplete())
//				return *it;
//		}
//	}
};


std::ostream & operator<<(std::ostream & o, tcpSession & s ) {
	o << "fd: " << s.getFd();
//	o << " buff: " << s.getRequest() << std::endl;
	return ( o );
}



