#pragma once
#include <unistd.h>
#include <fstream>
#include "Request.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstdlib>
#include <csignal>
#include <ctime>
#include <sys/stat.h> // chmod
#define IN 0
#define OUT 1

#define FILE 2
#define CGI  4
#define GENERIC 8
#define HTML 16
#define UPLOAD 32
#define FILE_ERROR 64
#define RESPONSE_TIMEOUT_ERROR 128

//#define STATE_DISCOVERED  //1
#define STATE_NONE  		  10  //2
#define STATE_WAIT  		  11  //2
#define STATE_INFC_CREATED    12  //3
#define STATE_INFC_PROCESSING 13  //3
#define STATE_DONE  		  14  //3
#define STATE_READY 		  15  //4
#define STATE_ERROR 		  16  //4
#define STATE_WAIT_PROC 	  17  //4

class CgiPipe;

class HttpResponse : public HttpParser {

  public:
	HttpResponse();
	HttpResponse(const HttpParser & copy);
	HttpResponse( const HttpResponse & copy );
	~HttpResponse( void ) ;

///		HttpResponse & operator=( const HttpResponse & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}
	// 1. store headers.
	// 2. store ready full response text
	std::string			response;
	std::string			response_body; // * temp var for body
	std::vector<BYTE>	response_buffer;
//	std::vector<BYTE>	cgi_resp_buffer; // todo
	std::string			file_type; // * file type for CGI
	int 				fd;	// * for reading file only
	int					pid;
	unsigned int		type;
	unsigned int		resp_state;
	bool				cgi_proc_exited;

	std::vector<BYTE> & get_response_buffer() ;
	std::string & get_response_body();
	unsigned int & get_resp_state();
	unsigned int & get_resp_type();

	std::string get_state_type_str() ;
	int read_from_file() ;
	void fill_buffer_to_send() ;
	int wait_process() ;
	int error_page_exists() ;
	int ready_to_write() ;
	void abort() ;
	bool been_sent() ;
	bool completed() const ;
	std::string get_host() ; // TODO

	int autoindex_directory(std::string & path) ;
	int search_file() ;
	std::string short_log_line() ;
	std::string get_response_status() ;
	void add_status_line() ;
	void add_headers() ;
	void generate_response_body() ;
	int create_file() ;
	int validate_path() ;
	void open_file_to_read(std::string & file) ;
	void init_response() ;
	bool does_need_interface() ;
	int spawn_process(std::vector<std::string> string_args) ;
	void set_environ_for_cgi() ;
	std::vector<std::string> create_cgi_args() ;
	int cgi() ;

};

