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
#define PUSHED 	  		 5

#define ADD_IFCE		 10
#define HANDLE_FILE 	 11

#define CRLF "\r\n"
#define BYTE char

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

	std::string		session_id;
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


	HttpParser( Server * serv );
	HttpParser( const HttpParser & copy );
	~HttpParser( void );
///		HttpParser & operator=( const HttpParser & other ) ;
	std::vector<BYTE> & get_request_buffer();
	std::map<std::string, std::string> & getHeaders();
	unsigned int & getState() ;

	bool 			isComplete() const ;
	bool 			beenPushed() const ;
	void 			setBeenPushed() ;
	unsigned int 	getCode() ;

	int 			setState(int macroState) ;
	void 			setReason(std::string r);

	int setCode(int val, std::string const & why="") ;
	void socket_eof();
	int parseInput(std::vector<BYTE> & input) ;
	std::vector<BYTE>::iterator find_CRLF(std::vector<BYTE> & input) ;
	int getHeaderLine(std::vector<BYTE> & input);
	std::string::size_type find_whitespace(std::string & str) ;
	void get_query_string() ;
	int parseStartLine() ;
	int validateMethodExists() ;
	int validateMethodImplemented() ;
	int parseHeaderLine() ;
	void match_config_and_location() ;
	int parseCookies(std::string & buffer) ;
	void validateHeaders() ;
	int find_chunk_size( std::vector<BYTE> & input ) ;
	int parse_chunked( std::vector<BYTE> & input ) ;

	int parseBody(std::vector<BYTE> & input) ;
	void display(std::ostream & o) ;
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
		HttpRequest(Server * serv);
};

//class HttpResponse : public HttpParser {
//	public:
//		HttpResponse( int fd ) : HttpParser() {}
//private:
//	std::string buffer;
//};
