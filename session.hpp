#include <iostream>
#pragma once
#define LISTENING_SESSION 0// zero stub for vector of FDs

class Session {
	private:
		int	 		fd; // 0 if listening fd
		std::string buffIn;
		std::string buffOut;
	public:
		Session( int fd ) : fd(fd) {}

		Session( const Session & copy ) : fd(copy.getFd()) {} //, buffer(copy.getBuff()) {}

		~Session( void ) {}

		Session & operator=( const Session & other ) {
			fd = other.getFd();
			buffIn = other.getBuffIn();
			buffOut = other.getBuffOut();
			return *this;
		}

		int 		getFd( void )     const { return fd; }
		std::string getBuffIn( void )   const { return buffIn; }
		std::string getBuffOut( void )   const { return buffOut; }
		bool 		toBeWrite( void ) const { return !buffOut.empty(); }
};

std::ostream & operator<<( std::ostream & o, Session & s ) {
	o << "fd: " << s.getFd();
	o << " buff: " << s.getBuffIn() << std::endl;
	return (o);
}

