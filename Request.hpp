#pragma once
#include <map>
#include <vector>
#include <algorithm>
//it is RECOMMENDED that all HTTP senders and recipients support, at a minimum, request-line lengths of 8000 octets.
#define BUFF_SIZE 10000
#define MAX_BUFFER_SIZE 10000
// flags
#define START_LINE 1
#define HEADER 	   2
#define BODY 	   3
#define DONE 	   4
#define ERROR 	  -1
enum HttpCodes {
	BAD_REQUEST = 400,
};

std::string HttpMethods[] = {"GET", "POST", "PUT", "CONNECT", "DELETE", "OPTIONS", "TRACE"};
std::string HttpMethodsImplemented[] = {"GET", "POST", "DELETE"};

void ltrim(std::string & s) {
	s.erase(s.begin(),
		    std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void rtrim(std::string & s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
					  std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void trim(std::string & s) {
	ltrim(s);
	rtrim(s);
}
//template <class T>
//bool in(T & val, T * arr) {
//	T * e = std::end(arr);
//	return e != std::find(std::begin(arr), e, val);
//}

bool isDigits(const std::string & str) {
	//https://stackoverflow.com/questions/8888748/how-to-check-if-given-c-string-or-char-contains-only-digits
	return str.find_first_not_of("0123456789") == std::string::npos;
}

bool isDigitsAndOws(const std::string & str) {
	return str.find_first_not_of("0123456789 \t") == std::string::npos;
}

class HttpParser {
	protected:
		std::string   buffer;
		std::string   bufExt;
		unsigned char state;
		unsigned int  code;
	// maybe better to store not splited headers
		std::map< std::string, std::string > headers;
		std::map< std::string, std::string >::iterator m;

		std::string method;
		std::string target;
		std::string version;
		unsigned long length;

	// maybe not good way to do it
		typedef std::pair <std::string, std::string> headers_pair;

	    bool 		  verbose;
	    int	 		  fd;
	public:
		HttpParser( int fd ) : fd(fd), state(0), length(0), verbose(true) { verbose && fd && std::cout << "HttpParser created, fd: " << fd << std::endl; }
		HttpParser( const HttpParser & copy ) { *this = copy; }
		~HttpParser( void ) { verbose && std::cout << "HttpParser destructed, fd: " << fd << std::endl; }
///		HttpParser & operator=( const HttpParser & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}
		std::string getBuffer() { return buffer; }

//		void 		  writeSocket( const std::string & str ) { buffer.append(str); }

	/// ------------------------------------------------

		int  		  writeSocket() {
			if (!bufExt.empty()) {
				::write(fd, bufExt.c_str(), bufExt.size());
			}
			bufExt.clear();
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
			if (rc < BUFF_SIZE || check_state())
				state |= BODY;

			bufExt.append( buff );
			parseInput(bufExt);
			return 1;
		}
		/// ------------------------------------------------

//		int checkState() {
//			return 1;
//		}

		int parseInput(std::string & input) {
			state < BODY 	    && carveHeaderLine(input);
			state == START_LINE	&& parseStartLine();
			state == HEADER 	&& parseHeaderLine();
			state == BODY		&& parseBody(input);
			if (isRequestDone())
				return 1;
			return 0;
		}

		int carveHeaderLine(std::string & input) {
			/// take header from input and return carved string
			size_t crlf_pos = input.find("\r\n");
			if (crlf_pos == std::string::npos || input.size() > MAX_BUFFER_SIZE) // headers only
				return 0;
			buffer = input.substr(0, crlf_pos);
			input.erase(0, crlf_pos + 2);
			if (buffer.empty()) {
				setState(BODY);
				validateHeaders();
				return 1;
			}
			return 1;
		}

		bool isRequestDone() const { return state > BODY; }

		int setState(int macroState) {
			state = macroState;
			return state;
		}

		int setCode(int val) {
			setState(DONE);
			code = val;
			return 1;  // just for returning smthing
		}

		/// im keeping that wired interface for future corrections and cause i like that : )
		int parseStartLine() {
			///	request-line   = method SP request-target SP HTTP-version // CRLF
			size_t pos = buffer.find(' ');
			method = buffer.substr(0, pos);

			if (!validateMethodExists())
				return setCode(400); // (Bad Request)
			if (!validateMethodImplemented())
				return setCode(501); // (Not Implemented)
			buffer.erase(0, pos + 1);

			pos = buffer.find(' ');
			target = buffer.substr(0, pos);
			buffer.erase(0, pos + 1);

			version = buffer.substr(0, pos);
			setState(HEADER);
			return 0;
		}

		int validateMethodExists() {
			std::string * e = std::end(HttpMethods);
			return e != std::find(std::begin(HttpMethods), e, method);
		}

		int validateMethodImplemented() {
			std::string * e = std::end(HttpMethodsImplemented);
			return e != std::find(std::begin(HttpMethodsImplemented), e, method);
		}

		int parseHeaderLine() {
			size_t pos = buffer.find(':');
			if (pos == std::string::npos)
				return 0;
			std::string key = buffer.substr(0, pos);
			buffer.erase(0, pos + 1);
			headers.insert(headers_pair(key, buffer));
			return 1;
		}

		//Transfer-Encoding
		void validateHeaders() {
			if (headers.find("Content-Length") != headers.end()) {
				m = headers.find("Content-Length");
				std::string val = m->second;
				trim(val);
				isDigits(val) || setCode(BAD_REQUEST);
				length = ::strtol(m->second.c_str(), NULL, 10);
			}
			if (headers.find("Transfer-Encoding") != headers.end()) {
				setCode(501);
			}
		}

		int parseBody(std::string & input) {}

};


class HttpRequest : public HttpParser {
	public:
		HttpRequest( int fd ) : HttpParser( fd ) {}
};

class HttpResponse : public HttpParser {
	public:
		HttpResponse( int fd ) : HttpParser( fd ) {}
//private:
//	std::string buffer;
};
