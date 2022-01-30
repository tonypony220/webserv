#pragma once
#include "Request.hpp"

class HttpResponse : public HttpParser {

  public:
//	HttpResponse() : HttpParser() { verbose && std::cout << "HttpResponse created"  << std::endl; }
	HttpResponse(const HttpParser & copy) : HttpParser(copy) { verbose && std::cout << "HttpResponse created"  << std::endl; }
	HttpResponse( const HttpResponse & copy ) { *this = copy; }
	~HttpResponse( void ) { verbose && std::cout << "HttpResponse destructed, fd: "  << std::endl; }
///		HttpResponse & operator=( const HttpResponse & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}

	bool isResponseDone() const { return false; }  //todo
};

