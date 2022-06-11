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

class IOInterface { // fd not assumed to be always  duplex
protected:
	int	 		 	fd; // 0 if listening fd
public: 
	IOInterface(int  fd_, Server * serv) 
		: fd(fd_), counter(0), server_ptr(serv) 
	{
		log("IOInterface created fd: ", fd);
	}
	size_t			counter;
	virtual			~IOInterface() {
		log(counter, " IOInterface destructed: ", fd);
	}
	int 		 	getFd( void ) const { return fd; }
	Server * 		getServ( void ) const { return server_ptr; }
	virtual int		processEvent( short event ) {};
	virtual IOInterface * get_interface() { return NULL; }

	Server *				  server_ptr;
};

class CgiPipe : public IOInterface {
public:
//	std::string		buffer;
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
		if (rc == 0) {
			response_ptr->get_resp_state() = STATE_DONE;
			return END;
		}
		if (rc < 0) {
			log(RED"read pipe error: " , strerror(errno), RESET);
			response_ptr->get_resp_state() = STATE_ERROR;
			return ERROR;
		}
		response_ptr->get_response_body().append( buff );
//		buffer.append( buff );
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

class File : public IOInterface {
public:
	HttpResponse *	response_ptr;

	File(int fd, HttpResponse * resp)
		: IOInterface(fd, NULL), response_ptr(resp) {} // NULL stub

	virtual int		processEvent( short event ) {
		int ret;

		std::string & buff = response_ptr->get_request_buffer();
		int 		rc = ::write(fd, buff.c_str(), buff.size());
		log("write to file ", fd, BLUE"bytes="RESET, rc); //fd &&
		if (rc < 0) {
			log(RED"write error: ", strerror(errno), RESET);
			return ERROR;
		}
		if (rc == 0) {
			response_ptr->get_resp_state() = STATE_DONE;
			return END;
		}

		buff.erase(0, rc);
		return SUCCESS;
	}
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

	IOInterface * get_interface() {
//	CgiPipe * 	 get_cgi_pipe() {
		if ( responses[current_response].get_resp_state() & CGI ) {
			int fd = responses[current_response].cgi();
			if (fd == ERROR)
				return NULL;
			return new CgiPipe(fd, &responses[current_response]);
		} else {
			int fd = responses[current_response].create_file();
			if (fd == ERROR)
				return NULL;
			return new File(fd, &responses[current_response]);
		}

	}

	int 		 processEvent( short event ) {
		int ret;
		if (event & POLLIN) {
			ret = readSocket();
//			log("\tsession reading...fd= ", fd, "read=", ret);
			if (ret == ERROR)
				return ERROR;
		}
//		log("\tresponses=", responses.size());
		if ( !responses.empty() ) {
//			responses[current_response].();
			if (responses[current_response].is_cgi()) {
				log(BLUE"session start create additional Interface..."RESET, fd);
				return HANDLE_CGI;
			}
			if (responses[current_response].ready_to_write()) {// && (event & POLLOUT) ) { // TCP buffer have space to write to
				log("\tsession writing...", fd);
				ret = writeSocket();
				if (ret == ERROR)
					return ERROR;
//				if (ret == END && responses[current_response].completed()) // TODO next response
//					return END;
			}
			if ( responses[current_response].completed() )
				return END;
		}
		return SUCCESS;
	}
	int  		  writeSocket() {
		//TODO maybe not sent all response for once cause
		// e
		std::string & buff = responses[current_response].get_response_buffer();
		int 		rc = ::write(fd, buff.c_str(), buff.size());
//		log("write to socket:", PURPLE, buff, RESET); //fd &&
		if (rc < 0) {
//			std::cerr << RED"write error: "RESET << strerror(errno);
			log(RED"write error: ", strerror(errno), RESET);
			return ERROR;
		} //if (rc == 0) { ????????????
		if (rc == 0) {
//			std::cerr << RED"write error: "RESET << strerror(errno);
			log(RED"idle write: ", RESET);
			return SUCCESS;
		} //if (rc == 0) { ????????????
		buff.erase(0, rc);
		if ( buff.empty() )
			return END;
		//responses[current_response].
		return SUCCESS;
	}
	int 		   readSocket() {
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		int rc = read(fd, buff, BUFF_SIZE - 1);
		if (rc == 0) {
			//std::cerr << RED"closed"RESET;
			return END; //TODO should return  END
		}
		if (rc < 0) {
			log(RED"read socket error: ",strerror(errno),RESET);
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



