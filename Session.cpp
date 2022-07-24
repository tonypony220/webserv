#include "Session.hpp"
#include <sys/poll.h>

	IOInterface::IOInterface(int  fd_, Server * serv)
		: fd(fd_), counter(0), server_ptr(serv)
	{
		log(PURPLE"IOInterface created fd: ", fd, RESET);
	}
	IOInterface::~IOInterface() {
		log(counter, PURPLE" IOInterface destructed: ", fd, RESET);
	}
	int 		 	IOInterface::getFd( void ) const { return fd; }
	Server * 		IOInterface::getServ( void ) const { return server_ptr; }
	int		IOInterface::processEvent( short event ) {
		event++;
		return SUCCESS ;
	}
	IOInterface * IOInterface::get_interface() { return NULL; }

CgiPipe::CgiPipe(int fd, HttpResponse * resp)
		: IOInterface(fd, NULL), response_ptr(resp) {
		log(CYAN" CGI Interface created, fd=", fd, RESET);
	} // NULL stub

CgiPipe::~CgiPipe() {
		log(CYAN" CGI Interface destructed, fd=", fd, RESET);
		close(fd);
	}

	int		CgiPipe::processEvent( short event ) {
		int ret;
		if (event & POLLIN) {
			ret = readPipe();
			if (ret == ERROR || ret == END)
				return ret;
		}
		return SUCCESS;
	}
	int 		   CgiPipe::readPipe() {
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		int rc = read(fd, buff, BUFF_SIZE - 1);
		log(PURPLE"\r\t\t\t\tprocessing cgi pipe read=", rc,RESET);
		if (rc == 0) {
			response_ptr->get_resp_state() = STATE_DONE;
			return END;
		}
		if (rc < 0) {
			log(RED"read pipe error: " , strerror(errno), RESET);
			response_ptr->get_resp_state() = STATE_DONE;
//			response_ptr->get_resp_state() = STATE_ERROR;
			return ERROR;
		}
		response_ptr->get_response_buffer().insert(
				response_ptr->get_response_buffer().end(),
				buff,
				buff + rc);
//		response_ptr->get_response_body().append( buff );
//		buffer.append( buff );
		return SUCCESS;
	}

	File::File(int fd, HttpResponse * resp)
		: IOInterface(fd, NULL), response_ptr(resp) {
		log(" FILE Interface created, fd=", fd);
	} // NULL stub
File::~File() {
		log(" FILE Interface destructed, fd=", fd);
		close(fd);
	}

	int		File::processEvent( short event ) {
		event++; // unused var :)
		std::vector<BYTE> & buff = response_ptr->get_request_buffer();
		int 		rc = ::write(fd, &buff[0], buff.size());
		log("write to file ", fd, BLUE"bytes="RESET, rc); //fd &&
		if (rc < 0) {
			log(RED"file write error: ", strerror(errno), RESET);
			return ERROR;
		}
		if (rc == 0) {
			response_ptr->get_resp_state() = STATE_DONE;
			return END;
		}

		buff.erase(buff.begin(), buff.begin() + rc);
		return SUCCESS;
	}

tcpSession::tcpSession(int fd, Server * serv)
		:
		IOInterface(fd, serv),
		current_request(0),
		current_response(0)
		{
			add_request();
			read_eof = false;
			start = std::time(nullptr);
			log(PURPLE"tcpSession created, fd: ", fd, RESET); //fd &&
		}

tcpSession::~tcpSession() {
		log(PURPLE"tcpSession destructed, fd: ", fd, RESET);
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
	void 		 tcpSession::add_request() {
		requests.push_back(HttpParser(server_ptr));
	}

	IOInterface * tcpSession::get_interface() {
//	CgiPipe * 	 get_cgi_pipe() {
		if ( responses[current_response].get_resp_type() & CGI ) {
			log(BLUE"creating additional Interface..CGI "RESET, fd);
			int fd = responses[current_response].cgi();
			if (fd == ERROR)
				return NULL;
			return new CgiPipe(fd, &responses[current_response]);
		} else {
			log(BLUE"creating additional Interface..FILE "RESET, fd);
			int fd = responses[current_response].create_file();
			if (fd == ERROR)
				return NULL;
			return new File(fd, &responses[current_response]);
		}

	}
	void tcpSession::add_response() {
		HttpResponse resp = HttpResponse(requests[current_request]);
		std::cout << resp << std::endl;
		requests[current_request].setBeenPushed();
		responses.push_back(resp);
		resp_start = std::time(nullptr);
	}

	int 		 tcpSession::processEvent( short event ) {
		int ret = SUCCESS;
		if (!read_eof && (event & POLLIN)) {
			/* EOF could be in middle of request, and this will make request done at once */
			ret = readSocket();
			if (ret == END) {
				read_eof = true;
				requests[current_request].socket_eof();
				ret = SUCCESS;
//				add_response();
			}
			if (requests[current_request].isComplete()
			&& !requests[current_request].beenPushed()) {
				add_response();
//				requests.clear();
				if (!read_eof) { /* keep-alive not works for now anyway */
					add_request();
//					current_request++;
				}
			}
		}
//		std::cout.precision(40);
//		std::cout << "difftime=" << difftime( time(nullptr), start ) << std::endl;
//		std::cout << "requests=" << requests.size() << std::endl;
//		std::cout << "responses=" << responses.size() << std::endl;
//		printf("%.f seconds have passed since the beginning of the month.\n", difftime( time(nullptr), start ));
		if (!requests[current_request].isComplete()
		&& difftime( time(nullptr), start ) > DEFAULT_REQUEST_TIMEOUT) {
			requests[current_request].setCode(
					HttpStatus::RequestTimeout, "timeout");
			add_response();
//			requests.clear();
		}
		// TODO
//		if (requests[current_request].isComplete() && !buffer.empty()) {
//			add_request();
//			current_request++;
//		}
//		log("\tresponses=", responses.size());
		if ( !responses.empty() ) {
//			responses[current_response].();
			if (responses[current_response].does_need_interface()) {
				log(BLUE"session should create additional Interface..."RESET, fd);
				return ADD_IFCE;
			}
			if (responses[current_response].ready_to_write()
			&& (event & POLLOUT) ) { // TCP buffer have space to write to
				std::cout << "\r\t\t\t\tsession writing fd(" << fd << ")";
				ret = writeSocket(); // not returns END !
				resp_start = std::time(nullptr);
//				if (ret == ERROR)
//					return ERROR;
			}
			if (ret == ERROR
			|| difftime( time(nullptr), resp_start ) > DEFAULT_RESPONSE_TIMEOUT) {
//				log(RED"Repsonse timeed out"RESET);
				responses[current_response].abort();
			}

			if ( responses[current_response].completed() ) {
				//127.0.0.1 - - [13/Jun/2022:19:19:54 +0300] "GET /file HTTP/1.1" 200 0 "-" "curl/7.54.0"
				std::string color(responses[current_response].been_sent() ? GREEN : RED);
				const std::time_t now = std::time(nullptr);
				char buf[64];
				strftime(buf, sizeof buf, "[%e/%b/%Y:%H:%M:%S %z]",
			 				std::localtime(&now));
				std::cout << "\n" << color << ip << " - " << buf
				<< responses[current_response].start_line << "  "
				<< responses[current_response].short_log_line()
				<< RESET << std::endl;
				std::cout.flush();
				return END;
			}
		}
		if ( ret )
			std::cout << RED"WTF";
		return ret;
//		return SUCCESS;
	}
	int  		  tcpSession::writeSocket() {
		//TODO maybe not sent all response for once cause
		std::vector<BYTE> & buff = responses[current_response].get_response_buffer();
		int 		rc = ::write(fd, &buff[0], buff.size());
		std::cout << "bytes=" << rc;
//		log("write to socket:", PURPLE, buff, RESET); //fd &&
		if (rc < 0) {
//			std::cerr << RED"write error: "RESET << strerror(errno);
			log(RED"socket write error: ", strerror(errno), RESET);
			return ERROR;
		} //if (rc == 0) { ????????????
		if (rc == 0) {
//			std::cerr << RED"write error: "RESET << strerror(errno);
			log(RED"idle write: ", RESET);
			return SUCCESS;
		} //if (rc == 0) { ????????????
		buff.erase(buff.begin(), buff.begin() + rc);
//		if ( buff.empty() )
//			return END;
		//responses[current_response].
		return SUCCESS;
	}
	int 		   tcpSession::readSocket() {
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		int rc = read(fd, buff, BUFF_SIZE - 1);
		log(BLUE"\tsession reading...fd= ", fd, "read="RESET, rc);
		if (rc == 0) {
			//std::cerr << RED"closed"RESET;
			return END; //TODO should return  END
//			return SUCCESS;
		}
		if (rc < 0) {
			log(RED"read socket error: ",strerror(errno),RESET);
			return ERROR;
		}
		start = std::time(nullptr);
		buffer.insert(buffer.end(), buff, buff + rc);
//		buffer.append( buff );
		requests[current_request].parseInput( buffer );

		return SUCCESS;
	}

//	HttpParser & getSessionToRead() {
//		for (sesstor it = messages.begin(); it != messages.end(); ++it) {
//			if (!it->isComplete())
//				return *it;
//		}
//	}


std::ostream & operator<<(std::ostream & o, tcpSession & s ) {
	o << "fd: " << s.getFd();
//	o << " buff: " << s.getRequest() << std::endl;
	return ( o );
}



