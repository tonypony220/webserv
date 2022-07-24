#pragma once
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "Request.hpp"
#include "Utils.hpp"
#include "Response.hpp"
#include "Server.hpp"
#define LISTENING_SESSION 0 // zero stub for vector of FDs
#define DEFAULT_REQUEST_TIMEOUT 4 // seconds
#define DEFAULT_RESPONSE_TIMEOUT 5 // seconds
//#define BUFF_SIZE 3
//#define MORE 1

class IOInterface { // fd not assumed to be always  duplex
protected:
	int	 		 	fd; // 0 if listening fd
public: 
	IOInterface(int  fd_, Server * serv);
	size_t			counter;
	virtual			~IOInterface() ;
	int 		 	getFd( void ) const;
	Server * 		getServ( void ) const ;
	virtual int		processEvent( short event ) ;
	virtual IOInterface * get_interface() ;

	Server *				  server_ptr;
};

class CgiPipe : public IOInterface {
public:
	HttpResponse * response_ptr;

	CgiPipe(int fd, HttpResponse * resp) ;
	~CgiPipe() ;

	virtual int		processEvent( short event ) ;
	int 		   readPipe() ;
};

class File : public IOInterface {
public:
	HttpResponse *	response_ptr; /* not protected from closing */

	File(int fd, HttpResponse * resp);
	~File() ;

	virtual int		processEvent( short event ) ;
};

class tcpSession : public IOInterface {
  private:
	//int	 		 			  fd; // 0 if listening fd
	std::vector<HttpParser>   requests;
	std::vector<HttpResponse> responses;
	unsigned int 			  current_request;
	unsigned int 			  current_response;
//	std::string 			  buffer;
	std::vector<char> 		  buffer;
	std::time_t 			  start; // = std::time(nullptr);
	std::time_t 			  resp_start; // = std::time(nullptr);
	bool					  read_eof;

public:
	std::string 			  ip;

	tcpSession(int fd, Server * serv);
	~tcpSession() ;
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
	
//	HttpRequest  getRequest( void )  const { return request; }
//	HttpResponse getResponse( void ) const { return response; }

	void 		 add_request() ;

	IOInterface * get_interface() ;
	void add_response() ;

	int 		 processEvent( short event ) ;
	int  		  writeSocket() ;
	int 		   readSocket() ;
};


std::ostream & operator<<(std::ostream & o, tcpSession & s ) ;



