#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "Utils.hpp"
#include "Server.hpp"
#include "HttpStatusCodes.hpp"
//it is RECOMMENDED that all HTTP senders and recipients support, at a minimum, request-line lengths of 8000 octets.
#define BUFF_SIZE 100000
#define MAX_HEADER_SIZE 1000
// flags
#define PARSE_START_LINE 1
#define HEADER 	  		 2
#define PARSE_BODY		 3
#define DONE 	  		 4

#define ADD_IFCE		 10
#define HANDLE_FILE 	 11

#define CRLF "\r\n"
#define BYTE char

//class Server_config;
//class Server;
//class Location;
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
	std::vector<BYTE> request_buffer;
//	std::string   	bufExt;
	unsigned int	state; // it was char but outputed nonprintable chars
	unsigned int 	code;
// maybe better to store not splited headers
	std::map< std::string, std::string > headers;
	std::map< std::string, std::string > cookies;

	size_t			session_id;
	std::string 	method;
	std::string 	target;
	std::string		path;
	std::string		query_string;
	std::string 	version;
	std::string 	protocol;
	std::string 	transfer_encoding;
	unsigned long 	length; // content lenght
	unsigned long 	counter;
	size_t 			chunk_size; // current chunk size 
	bool			chunk_size_parsed;
	Server_config	*config;
	Location		*location;

	// maybe not good way to do it
	typedef std::map< std::string, std::string >::iterator headerItor;
	typedef std::pair <std::string, std::string> headersPair;

	bool 		  verbose;
//	    int	 		  fd;
  public:
	std::string	  start_line; // for logger
	std::string   debug_info;
	Server *	  server_ptr;


	HttpParser( Server * serv ) 
		: state(PARSE_START_LINE),
		  length(0), 
		  verbose(true),
		  code(0), 
		  counter(0),
		  session_id(0),
		  chunk_size_parsed(false),
		  server_ptr(serv),
		  config(serv->get_default_config()) {
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
	std::vector<BYTE> & get_request_buffer() { return request_buffer; }
	std::map<std::string, std::string> & getHeaders() { return headers; }
	unsigned int & getState() { return state; }

	bool isComplete() const { return state > PARSE_BODY; }
	unsigned int getCode() { return code; }

	int setState(int macroState) {
		state = macroState;
		return state;
	}
	void setReason(std::string r) { debug_info = r; }

	int setCode(int val, std::string const & why="") {
		// setting response code
		log(BLUE"resp code set: ", code, RESET);
		log(BLUE"reason: ", why, RESET);
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

	int parseInput(std::vector<BYTE> & input) {
		counter++;
		//verbose && std::cout << "parsing.." << std::endl;
		while ( !input.empty() && !isComplete() ) {
//			verbose && std::cout << "input buffer: " << input << std::endl;
			//buffer.clear();	
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

	// TODO may be non-class-member
	std::vector<BYTE>::iterator find_CRLF(std::vector<BYTE> & input) {
		std::vector<BYTE>::iterator start = input.begin();
		std::vector<BYTE>::iterator res = find(start, input.end(), '\r');
		if (  res + 1 == input.end() )
			return input.end();
		while ( res != input.end() ) {
			if ( *(res + 1) == '\n' )
				return res;
			start = res + 1;
			res = find(start, input.end(), '\r');
		}
		return res;
	}

	int getHeaderLine(std::vector<BYTE> & input) {
		/// take header from input and return carved string
	//	verbose && std::cout << "INPUT\""BLUE << std::endl << input << RESET"\"" << std::endl;
	//  verbose && std::cout << "carving.."  << std::endl;
	//  verbose && std::cout << "pos"BLUE << crlf_pos << RESET << std::endl;
		std::vector<BYTE>::iterator crlf = find_CRLF(input);
//		size_t crlf_pos = input.find("\r\n");
//		if (crlf_pos == std::string::npos) {
		if (crlf == input.end()) {
			verbose && std::cout << "no CRLF" << std::endl;
			return END;
		}

//		verbose && std::cout << "input before carve: "CYAN << input << RESET << std::endl;
//		buffer = input.substr(0, crlf_pos);  // "content-lenght 40 CRLF
		buffer.assign(input.begin(), crlf);
		verbose && std::cout << "header line: "BLUE << buffer << RESET << std::endl;
		// TODO not correct
		if ( buffer.size() > MAX_HEADER_SIZE ) {
			setCode(HttpStatus::URITooLong);
			return END; // on error our request is complete
		}
		//verbose && std::cout << "LINE: '"CYAN << buffer << RESET"'" << std::endl;
//		verbose && std::cout << "pos"BLUE << crlf_pos << RESET << std::endl;
		input.erase(input.begin(), crlf + 2);
//		input.erase(0, crlf_pos + 2);
//		verbose && std::cout << "input after erace: "CYAN << input << RESET << std::endl;
		if (buffer.empty()) {
			//std::cout << BLUE;
			//display(std::cout);
			//std::cout << RESET << '\n';
			setState(PARSE_BODY);
			validateHeaders();
		}
		return SUCCESS;
	}

	std::string::size_type find_whitespace(std::string & str) {
		std::string::size_type pos = str.find(' ');
		if ( pos != std::string::npos )
			return pos;
		return str.find('\t');
		//return pos;
	}
	/// im keeping that wired interface for future corrections and cause i like that : )
	void get_query_string() {
		size_t pos = target.find("?");
		if ( pos == std::string::npos )
			return;
		query_string = target.substr(pos + 1, target.size() - 1);
		target = target.substr(0, pos);
	}
	int parseStartLine() {
		log(BLUE"parsing start line.. "RESET);
		///	request-line   = method SP request-target SP HTTP-version // CRLF
		start_line = buffer;
		size_t pos = find_whitespace(buffer);
		if ( pos == std::string::npos )
			return setCode(HttpStatus::BadRequest, "whitespace");
		method = buffer.substr(0, pos);

		if (!validateMethodExists())
			return setCode(HttpStatus::BadRequest, "method not exist");
		if (!validateMethodImplemented())
			return setCode(HttpStatus::NotImplemented, "not implemented"); // TODO
		buffer.erase(0, pos + 1);

		pos = find_whitespace(buffer);
		if ( pos == std::string::npos )
			return setCode(HttpStatus::BadRequest, "method not exist");
		target = buffer.substr(0, pos);
		get_query_string();
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
			setCode(HttpStatus::BadRequest, "Space before key in header");
			return ERROR;
		}
		if (pos == std::string::npos) {
			setCode(HttpStatus::BadRequest, "A sender MUST NOT generate a message that includes line folding");
			return ERROR;
		}
		std::string key = buffer.substr(0, pos);
		//std::cout << ">>>>>> '" << key << "'" << std::endl;
		if ( find_whitespace(key) != std::string::npos ) {
			setCode(HttpStatus::BadRequest, "No whitespace is allowed between the header field-name and colon");
			return ERROR;
		}
		to_lower(key);
		buffer.erase(0, pos + 1);
		trim(buffer);
		if ( unpack_dequtes(buffer) == EXIT_FAILURE ) {
			setCode(HttpStatus::BadRequest, "Bad field or quote");
			return ERROR;
		}
		if (key == "cookie")
			return parseCookies(buffer); // ERROR | SUCCESS;
		else
			headers.insert(headersPair(key, buffer));
		return SUCCESS;
	}
	void match_config_and_location() {
		std::string host(headers["host"]);
		std::string::size_type pos(host.find(":"));
		int port = 0;
		if ( host.empty() ) {
			port = atoi(host.substr(pos + 1, host.size()).c_str());
			host = host.substr(0, pos);
		}
		config = server_ptr->match_config( host, port );
		path = target;
		location = config->route_target_path(path);
		clear_path(path);
//		path.erase(0, 1);
		if ( host.empty() ) return;
		if (!location->method_allowed(method))
			setCode(HttpStatus::MethodNotAllowed, "not allowed");
		if (location->redirect_uri.size())
			setCode(HttpStatus::TemporaryRedirect, "redirect");
	}
//	void match_config_and_location() {
//		std::string host(headers["host"]);
//		std::string::size_type pos(host.find(":"));
//		config = server_ptr->match_config(host.substr(0, pos),
//										  atoi(host.substr(
//												  pos + 1, host.size()).c_str())
//		);
//		path = target;
//		location = config->route_target_path(path);
//		path.erase(0, 1);
//		if (!location->method_allowed(method))
//			setCode(HttpStatus::MethodNotAllowed, "not allowed");
//		if (location->redirect_uri.size())
//			setCode(HttpStatus::TemporaryRedirect, "redirect");
//	}

	// a=f
	// a=f ; b=c
	// af
	// af ; v = 4
	// ==== ;
	int parseCookies(std::string & buffer) {
		std::string::size_type colon = buffer.find(";");
		std::string::size_type eq = buffer.find("=");
		size_t 				   start = 0;

		if (eq == std::string::npos)
			return ERROR;
		while (eq != std::string::npos) {
			if (colon != std::string::npos && eq > colon)
				return ERROR;
			size_t end = buffer.size();
			if (colon != std::string::npos)
				end = colon;
			if ( eq-start==0 || end-eq==0 )
				return ERROR;
			std::string key = buffer.substr(start, eq-start);
			std::string val = buffer.substr(eq + 1,  end-eq);
			if (!cookies.insert(headersPair(key, val)).second)
				return ERROR; // duplicate
			start = end;
			colon = buffer.find(";", start);
			eq = buffer.find("=", start);
		}
		return SUCCESS;
	}

	void validateHeaders() {
		headerItor te = headers.find("transfer-encoding");
		headerItor cl = headers.find("content-length");
		match_config_and_location();

		if (te != headers.end() && cl != headers.end()) {
			setCode(HttpStatus::BadRequest, "both content-length and encoding headers");
			return ;
		}
//		headerItor cookies_it = cookies.find("id");

//		if ( cookies_it != cookies.end() ) {
//			session_id = server_ptr->parse_session_id(cookies_it->second);
//		}
		if ( headers.find("host") == headers.end() ) {
			setCode(HttpStatus::BadRequest, "host header is missing");
			return ;
		}
		if (cl != headers.end()) {
//			cl = headers.find("Content-Length");
			std::string val = cl->second;
			trim(val);
			isDigits(val) || setCode(HttpStatus::BadRequest);
			length = ::strtol(cl->second.c_str(), NULL, 10);
		}
		if (te != headers.end() ) { 
			if ( te->second != "chunked" )
				setCode(HttpStatus::NotImplemented);
			else 
				transfer_encoding = "chunked"; 
		} else 
			length == 0 && setState(DONE); // TODO

	}
	int find_chunk_size( std::vector<BYTE> & input ) {
		std::vector<BYTE>::iterator crlf = find_CRLF(input);
		if (crlf == input.end()) {
			log(YELLOW"chunk size not complete yet"RESET);
			return SUCCESS;
		}
		std::string::size_type pos;
//		if (pos == std::string::npos) {
//			log(YELLOW"chunk size not complete yet"RESET);
//			return SUCCESS;
//		}
		std::string chunk_head(input.begin(), crlf);
		input.erase(input.begin(), crlf + 2);
//		input.erase(0, pos + 2);
		pos = chunk_head.find(';');
		if (pos != std::string::npos) {
			log(BLUE"chunk extension found"RESET);
			std::string extensions(chunk_head.substr(pos + 1, chunk_head.size() - pos - 1));
			chunk_head = chunk_head.substr(0, pos);
		}
		if ( !validate_hex(chunk_head) ) {
			log(RED"hex size incorrect"RESET);
			return ERROR;
		}
		chunk_size = hextoi(chunk_head);
		log("size parsing..");
		chunk_size_parsed = true;
		return SUCCESS;
		//}
	}
	/*
	chunk          = chunk-size [ chunk-ext ] CRLF
                     chunk-data CRLF
    chunk-size     = 1*HEXDIG
    last-chunk     = 1*("0") [ chunk-ext ] CRLF
	chunk-ext      = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
    chunk-ext-name = token
    chunk-ext-val  = token / quoted-string
    */ // TODO trailer part
	int parse_chunked( std::vector<BYTE> & input ) {
		log("parsing chunked. size found=", chunk_size_parsed);
		if ( !chunk_size_parsed && find_chunk_size(input) == ERROR ) {
			log(RED"chunked size error"RESET);
			return ERROR;
		}
		if (chunk_size_parsed) log(YELLOW"chunked size="RESET, chunk_size);
		if ( chunk_size == 0 )
			return END;  // for now we ignore trailer part
		if ( input.size() >= chunk_size + 2 ) {
			log(PURPLE"getting full chunk"RESET);
//			std::vector<BYTE>::iterator crlf = find_CRLF(input);
//			std::string::size_type pos = input.find(CRLF, chunk_size);
//			log(PURPLE"crlf pos = ", crlf - input.begin(), RESET);
			if ( input[chunk_size] != '\r' || input[chunk_size+1] != '\n') {
				log(RED"chunked CRLF error", RESET);
				return ERROR;
			}
			request_buffer.insert(request_buffer.end(),
								  input.begin(),
								  input.begin() + chunk_size
			);
//			buffer.append(input.begin(), input.begin() + pos);
//			input.erase(0, pos + 2);
			input.erase(input.begin(), input.begin() + chunk_size + 2);
//			log("request buffer: ", buffer);
			chunk_size_parsed = false;
		} else {
			log("getting part chunk");
			request_buffer.insert(request_buffer.end(),
								  input.begin(),
								  input.end()
			);
//			buffer.append(input);
			chunk_size -= input.size();
			input.clear();
		}
		return SUCCESS;
	}

	int parseBody(std::vector<BYTE> & input) {
		std::cout << "\rbody consuming, input buffer size=" << input.size();
		std::cout << " len=" << length << " / buffsize=" << buffer.size()  << " - " << '\n';
//		std::cout << " data: "CYAN << input << RESET << std::endl;
		if (config->max_size && request_buffer.size() > config->max_size ) {
			setCode(HttpStatus::ContentTooLarge, "size > limit");
			return END;
		}
		if ( !transfer_encoding.size() ) {
			if ( request_buffer.size() + input.size() >= length ) {
				request_buffer.insert(request_buffer.end(),
									  input.begin(),
									  input.begin() + length - request_buffer.size()
									  );
//				buffer.append( input.substr(0, length - buffer.size()) );
				setState(DONE);
				// should be error not  correct data?
			} else {
				request_buffer.insert(request_buffer.end(),
									  input.begin(),
									  input.end()
									  );
//				buffer.append(input);
			}
			input.clear();
		} else {
			int ret = parse_chunked(input);
			if ( ret == END )
				setState(DONE);
			else if ( ret == ERROR ) 
				setCode( HttpStatus::BadRequest, "err" );
		}
		return SUCCESS;
	}

	void display(std::ostream & o) {
		o << YELLOW"HttpParser(" << std::endl;
		o << "\tcode: "    << code << std::endl;
		o << "\tstate: "   << state << std::endl;
		o << "\tmethod: "  << method << std::endl;
		o << "\tversion: " << version << std::endl;
		o << "\tdebug_info: " << debug_info << std::endl;
		o << "\ttarget: "  << target << std::endl;
		o << "\tquery: "  << query_string << std::endl;
		o << "\theaders number: " << headers.size() << std::endl;
		o << "\theaders: " << std::endl;
//		std::map< std::string, std::string>::iterator it;
		for (headerItor it = headers.begin(); it != headers.end(); it++) {
			o << "\t\t" << it->first << ": " << it->second << std::endl;
		}
		o << "\tcookies: " << std::endl;
//		std::map< std::string, std::string>::iterator it;
		for (headerItor it = cookies.begin(); it != cookies.end(); it++) {
			o << "\t\t" << it->first << ": " << it->second << std::endl;
		}
		o << "\tcounter:" << counter << std::endl;
		o << "\tbuffer:" << buffer << std::endl;
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
