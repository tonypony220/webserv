#pragma once
#define BUFF_SIZE 200

class HttpSession {
	protected:
		std::string   buffer;
		unsigned char complete;
	    bool 		  verbose;
	    int	 		  fd;
	public:
		HttpSession( int fd ) : fd(fd), complete(0), verbose(true) { verbose && fd && std::cout << "HttpSession created, fd: " << fd << std::endl; }
		HttpSession( const HttpSession & copy ) { *this = copy; }
		~HttpSession( void ) { verbose && std::cout << "HttpSession destructed, fd: " << fd << std::endl; }
///		HttpSession & operator=( const HttpSession & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}
		std::string getBuffer() { return buffer; }

//		void 		  writeSocket( const std::string & str ) { buffer.append(str); }

		int  		  writeSocket() {
			if (!buffer.empty()) {
				::write(fd, buffer.c_str(), buffer.size());
			}
			buffer.clear();
			return 1;
		}

		int 		   readSocket() {
			char buff[BUFF_SIZE];

			memset(buff, 0, BUFF_SIZE);
			int rc = ::read(fd, buff, BUFF_SIZE);
			if (rc <= 0) {
				if (rc < 0)
					std::cerr << "read error: " << strerror(errno);
				return 0;
			}
			if (rc < BUFF_SIZE || check_completed())
				complete++;
			buffer.append( buff );
			return 1;
		}

		int check_completed() {
			return 1;
		}

};

class HttpRequest : public HttpSession {
	public:
		HttpRequest( int fd ) : HttpSession( fd ) {}
};

class HttpResponse : public HttpSession {
	public:
		HttpResponse( int fd ) : HttpSession( fd ) {}
//private:
//	std::string buffer;
};
