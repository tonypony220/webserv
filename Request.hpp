#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "Utils.hpp"
#include "HttpStatusCodes.hpp"
//it is RECOMMENDED that all HTTP senders and recipients support, at a minimum, request-line lengths of 8000 octets.
#define BUFF_SIZE 10000
#define MAX_BUFFER_SIZE 10000
// flags
#define START_LINE 1
#define HEADER 	   2
#define BODY 	   3
#define DONE 	   4
#define ERROR 	  -1

# define RED 	"\033[1;31m"
# define GREEN   "\033[0;32m"
# define YELLOW	"\033[1;33m"
# define BLUE	"\033[1;34m"
# define PURPLE  "\033[1;35m"
# define CYAN    "\033[1;36m"
# define RESET   "\033[0;0m"
# define BOLD    "\033[;1m"
# define REVERSE "\033[;7m"

std::string HttpMethods[] = {"GET", "POST", "PUT", "CONNECT", "DELETE", "OPTIONS", "TRACE"};
std::string HttpMethodsImplemented[] = {"GET", "POST", "DELETE"};

class HttpParser {
  protected:
	std::string   	buffer;
//	std::string   	bufExt;
	unsigned int	state; // it was char but outputed nonprintable chars
	unsigned int 	code;
// maybe better to store not splited headers
	std::map< std::string, std::string > headers;

	std::string 	method;
	std::string 	target;
	std::string 	version;
	unsigned long 	length;

	// maybe not good way to do it
	typedef std::map< std::string, std::string >::iterator headerItor;
	typedef std::pair <std::string, std::string> headersPair;

	bool 		  verbose;
//	    int	 		  fd;
  public:
	std::string   reason;
	HttpParser( void ) : state(START_LINE), length(0), verbose(true), code(0) { verbose && std::cout << "HttpParser created, fd: "  << std::endl; }
	HttpParser( const HttpParser & copy ) { *this = copy; }
	~HttpParser( void ) { verbose && std::cout << "HttpParser destructed, fd: "  << std::endl; }
///		HttpParser & operator=( const HttpParser & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}
	std::string getBuffer() { return buffer; }
	std::map<std::string, std::string> & getHeaders() { return headers; }

	bool isRequestDone() const { return state > BODY; }
	unsigned int getCode() { return code; }

	int setState(int macroState) {
		state = macroState;
		return state;
	}
	void setReason(std::string r) { reason = r; }

	int setCode(int val, std::string const & why="") {
		setState(DONE);
		if (!why.empty())
			setReason(why);
		code = val;
		return 1;  // just for returning smthing
	}
//		void 		  writeSocket( const std::string & str ) { buffer.append(str); }

/// ------------------------------------------------

/// ------------------------------------------------
//		int checkState() {
//			return 1;
//		}

	int parseInput(std::string & input) {
//		verbose && std::cout << "parsing.." << std::endl;
		while (!input.empty() && !isRequestDone()) {
			verbose && std::cout << "parsing input buffer: "BLUE << input << RESET << std::endl;
//			verbose && std::cout << "input buffer: " << input << std::endl;
			if (state < BODY && !carveHeaderLine(input))
				return 0;
			if (state == START_LINE && !parseStartLine())
				continue;
			if (state == HEADER && parseHeaderLine())
				continue;
			state == BODY && parseBody(input);
//			if (isRequestDone())
//				return 1;
		}
		return 1;
	}

	int carveHeaderLine(std::string & input) {
		/// take header from input and return carved string
		verbose && std::cout << "carving input buffer:"BLUE << std::endl << input << RESET << std::endl;
//		verbose && std::cout << "carving.."  << std::endl;

		size_t crlf_pos = input.find("\r\n");
		verbose && std::cout << "pos"BLUE << crlf_pos << RESET << std::endl;
		if (crlf_pos == std::string::npos) {
			verbose && std::cout << "no CRLF" << std::endl;
			return 0;
		}

		if ( input.size() > MAX_BUFFER_SIZE )  {
			setCode(HttpStatus::URITooLong); // todo not correct
			return 0;
		}

//		verbose && std::cout << "buffer before carve: "RED << buffer << RESET << std::endl;
//		verbose && std::cout << "input before carve: "CYAN << input << RESET << std::endl;
		buffer = input.substr(0, crlf_pos);
//		verbose && std::cout << "carved buff: "CYAN << buffer << RESET << std::endl;
//		verbose && std::cout << "pos"BLUE << crlf_pos << RESET << std::endl;
		input.erase(0, crlf_pos + 2);
//		verbose && std::cout << "input after erace: "CYAN << input << RESET << std::endl;
		if (buffer.empty()) {
			setState(BODY);
			validateHeaders();
			return 1;
		}
		return 1;
	}

	/// im keeping that wired interface for future corrections and cause i like that : )
	int parseStartLine() {
		verbose && std::cout << "start line.. "  << std::endl;
		///	request-line   = method SP request-target SP HTTP-version // CRLF
		size_t pos = buffer.find(' ');
		method = buffer.substr(0, pos);

		if (!validateMethodExists())
			return setCode(HttpStatus::BadRequest, "method not exist");
		if (!validateMethodImplemented())
			return setCode(HttpStatus::NotImplemented, "not implemented"); // todo
		buffer.erase(0, pos + 1);

		pos = buffer.find(' ');
		target = buffer.substr(0, pos);
		buffer.erase(0, pos + 1);

		version = buffer;
		buffer.clear();
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
		verbose && std::cout << "parsing header line.. "  << std::endl;
		size_t pos = buffer.find(':');
		if (::isspace(buffer[0])) {
			setCode(HttpStatus::BadRequest, "space on top");
			return 0;
		}
		if (pos == std::string::npos) {
			setCode(HttpStatus::BadRequest, "no colon");
			return 0;
		}
		std::string key = (buffer.substr(0, pos));
		to_lower(key);
		buffer.erase(0, pos + 1);
		trim(buffer);
		headers.insert(headersPair(key, buffer));
		return 1;
	}

	void validateHeaders() {
		headerItor te = headers.find("transfer-encoding");
		headerItor cl = headers.find("content-length");

		if (te != headers.end() && cl != headers.end()) {
			setCode(HttpStatus::BadRequest);
			return ;
		}
		if (cl != headers.end()) {
//			cl = headers.find("Content-Length");
			std::string val = cl->second;
			trim(val);
			isDigits(val) || setCode(HttpStatus::BadRequest);
			length = ::strtol(cl->second.c_str(), NULL, 10);
		}
		if (te != headers.end()) {
			setCode(HttpStatus::NotImplemented);
		}
		length == 0 && setState(DONE);
	}

	int parseBody(std::string & input) {
		verbose && std::cout << "body consuming..." << std::endl;
		buffer.append(input);
		input.clear();
	}

	void display(std::ostream & o) {
		o << YELLOW"HttpParser(" << std::endl;
		o << "\tcode: "    << code << std::endl;
		o << "\tstate: "   << state << std::endl;
		o << "\tmethod: "  << method << std::endl;
		o << "\tversion: " << version << std::endl;
		o << "\ttarget: "  << target << std::endl;
		o << "\theaders number: " << headers.size() << std::endl;
		o << "\theaders: " << std::endl;
//		std::map< std::string, std::string>::iterator it;
		for (headerItor it = headers.begin(); it != headers.end(); it++) {
			o << "\t\t" << it->first << ": " << it->second << std::endl;
		}
		o << "buffer:" << buffer << std::endl;
		o << RESET << std::endl;
	}
};


std::ostream & operator<<( std::ostream & o, HttpParser & s ) {
//	std::map< std::string, std::string > & headers = s.getHeaders();
//	o << "HttpParser(" << s.getBuffer();
//	o << "buffer: " << s.getBuffer();
//	o << " code: " << s.getCode() << " why: " << s.reason << std::endl;
//	o << "headers: " << std::endl;
//	std::map< std::string, std::string>::iterator it;
//	for (it = headers.begin(); it != headers.end(); it++) {
//		o << it->first << ": " << it->second << std::endl;
//	}
	s.display(o);
//	o << ")" << s.getBuffer();
	return ( o );
}


class HttpRequest : public HttpParser {
	public:
		HttpRequest() : HttpParser() {}
};

//class HttpResponse : public HttpParser {
//	public:
//		HttpResponse( int fd ) : HttpParser() {}
//private:
//	std::string buffer;
//};
