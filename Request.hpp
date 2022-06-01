#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "Utils.hpp"
#include "Server.hpp"
#include "HttpStatusCodes.hpp"
//it is RECOMMENDED that all HTTP senders and recipients support, at a minimum, request-line lengths of 8000 octets.
#define BUFF_SIZE 10000
#define MAX_BUFFER_SIZE 10000
// flags
#define PARSE_START_LINE 1
#define HEADER 	  		 2
#define PARSE_BODY		 3
#define DONE 	  		 4
#define HANDLE_CGI		 2

#define ERROR 	  		-1
#define SUCCESS 		 0
#define END 			 1


std::string HttpMethods[] = {"GET", "POST", "PUT", "CONNECT", "DELETE", "OPTIONS", "TRACE"};
std::string HttpMethodsImplemented[] = {"GET", "POST", "DELETE"};
/*  *** Examples *** :
Client request:

     GET /hello.txt HTTP/1.1
     User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3
     Host: www.example.com
     Accept-Language: en, mi

Server response:

     HTTP/1.1 200 OK
     Date: Mon, 27 Jul 2009 12:28:53 GMT
     Server: Apache
     Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
     ETag: "34aa387-d-1568eb00"
     Accept-Ranges: bytes
     Content-Length: 51
     Vary: Accept-Encoding
     Content-Type: text/plain

     Hello World! My payload includes a trailing CRLF.
*/

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
	std::string 	protocol;
	unsigned long 	length; // content lenght

	// maybe not good way to do it
	typedef std::map< std::string, std::string >::iterator headerItor;
	typedef std::pair <std::string, std::string> headersPair;

	bool 		  verbose;
//	    int	 		  fd;
  public:
	std::string   debug_info;
	Server *	  server_ptr;


	HttpParser( Server * serv ) 
		: state(PARSE_START_LINE),
		  length(0), 
		  verbose(true), 
		  code(0), 
		  server_ptr(serv) {
		verbose && std::cout << "HttpParser created"  << std::endl; 
	}
	HttpParser( const HttpParser & copy ) { *this = copy; }
	~HttpParser( void ) { 
		verbose && std::cout << "HttpParser destructed"  << std::endl;
	}
///		HttpParser & operator=( const HttpParser & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}
	std::string getBuffer() { return buffer; }
	std::map<std::string, std::string> & getHeaders() { return headers; }

	bool isComplete() const { return state > PARSE_BODY; }
	unsigned int getCode() { return code; }

	int setState(int macroState) {
		state = macroState;
		return state;
	}
	void setReason(std::string r) { debug_info = r; }

	int setCode(int val, std::string const & why="") {
		// setting response code
		setState(DONE);
		if (!why.empty())
			setReason(why);
		code = val;
		return ERROR;  // just for returning smthing
	}
//		void 		  writeSocket( const std::string & str ) { buffer.append(str); }

/// ------------------------------------------------

/// ------------------------------------------------
//		int checkState() {
//			return 1;
//		}

	int parseInput(std::string & input) {
//		verbose && std::cout << "parsing.." << std::endl;
		while ( !input.empty() && !isComplete() ) {
			//verbose && std::cout << "parsing input buffer: "BLUE << input << RESET << std::endl;
			//verbose && std::cout << "input buffer: " << input << std::endl;
			buffer.clear();	
			if (state < PARSE_BODY && getHeaderLine(input) == END)
				break; // in case of error or not full header line
			if (state == PARSE_START_LINE && parseStartLine() == SUCCESS)
				continue;
			if (state == HEADER && parseHeaderLine() == SUCCESS)
				continue;
			state == PARSE_BODY && parseBody(input);
//			if (isComplete())
//				return 1;
		}
		return END; // does not matter for now
	}

	int getHeaderLine(std::string & input) {
		/// take header from input and return carved string
	//	verbose && std::cout << "INPUT\""BLUE << std::endl << input << RESET"\"" << std::endl;
	//verbose && std::cout << "carving.."  << std::endl;

		size_t crlf_pos = input.find("\r\n");
		//verbose && std::cout << "pos"BLUE << crlf_pos << RESET << std::endl;
		if (crlf_pos == std::string::npos) {
			verbose && std::cout << "no CRLF" << std::endl;
			return END;
		}
		if ( input.size() > MAX_BUFFER_SIZE ) {
			setCode(HttpStatus::URITooLong); // TODO not correct
			return END; // on error our request is complete
		}

//		verbose && std::cout << "buffer before carve: "RED << buffer << RESET << std::endl;
//		verbose && std::cout << "input before carve: "CYAN << input << RESET << std::endl;
		buffer = input.substr(0, crlf_pos);  // "content-lenght 40 CRLF
		//verbose && std::cout << "LINE: '"CYAN << buffer << RESET"'" << std::endl;
//		verbose && std::cout << "pos"BLUE << crlf_pos << RESET << std::endl;
		input.erase(0, crlf_pos + 2);
//		verbose && std::cout << "input after erace: "CYAN << input << RESET << std::endl;
		if (buffer.empty()) {
			setState(PARSE_BODY);
			validateHeaders();
		}
		return SUCCESS;
	}

	/// im keeping that wired interface for future corrections and cause i like that : )
	int parseStartLine() {
		verbose && std::cout << "start line.. "  << std::endl;
		///	request-line   = method SP request-target SP HTTP-version // CRLF
		size_t pos = buffer.find(' ');
		if ( pos == std::string::npos )
			return setCode(HttpStatus::BadRequest, "method not exist");
		method = buffer.substr(0, pos);

		if (!validateMethodExists())
			return setCode(HttpStatus::BadRequest, "method not exist");
		if (!validateMethodImplemented())
			return setCode(HttpStatus::NotImplemented, "not implemented"); // TODO
		buffer.erase(0, pos + 1);

		pos = buffer.find(' ');
		if ( pos == std::string::npos )
			return setCode(HttpStatus::BadRequest, "method not exist");
		target = buffer.substr(0, pos);
		buffer.erase(0, pos + 1);

		// buffer: HTTP/1.1
		pos = buffer.find('/');
		if ( pos == std::string::npos )
			return setCode(HttpStatus::BadRequest, "method not exist");
		protocol = buffer.substr(0, pos);
		if ( protocol != "HTTP" )
			return setCode(HttpStatus::NotFound, "not supported protocol");
		buffer.erase(0, pos + 1);
		version = buffer;
		if ( !( version == "1.1" || version == "1.0" ))
			return setCode(HttpStatus::HTTPVersionNotSupported, "not supported version");
		setState(HEADER);
		return SUCCESS;
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
		//verbose && std::cout << "parsing header line.. "  << std::endl;
		size_t pos = buffer.find(':');
		if (::isspace(buffer[0])) {
			setCode(HttpStatus::BadRequest, "space on top");
			return ERROR;
		}
		if (pos == std::string::npos) {
			setCode(HttpStatus::BadRequest, "no colon");
			return ERROR;
		}
		std::string key = (buffer.substr(0, pos));
		to_lower(key);
		buffer.erase(0, pos + 1);
		trim(buffer);
		headers.insert(headersPair(key, buffer));
		return SUCCESS;
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
		if ( buffer.size() + input.size() >= length ) {
			buffer.append(input.substr(0, length - buffer.size()));
			setState(DONE);
		}
		else 
			buffer.append(input);
		input.clear();
		return 0;
	}

	void display(std::ostream & o) {
		o << YELLOW"HttpParser(" << std::endl;
		o << "\tcode: "    << code << std::endl;
		o << "\tstate: "   << state << std::endl;
		o << "\tmethod: "  << method << std::endl;
		o << "\tversion: " << version << std::endl;
		o << "\tdebug_info: " << debug_info << std::endl;
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
//	o << " code: " << s.getCode() << " why: " << s.debug_info << std::endl;
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
		HttpRequest(Server * serv) : HttpParser(serv) {}
};

//class HttpResponse : public HttpParser {
//	public:
//		HttpResponse( int fd ) : HttpParser() {}
//private:
//	std::string buffer;
//};
