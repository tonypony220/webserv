#include <iostream>

class Session {
	private:
		int	 fd;
		std::string buffIn;
		std::string buffOut;
public:
		Session( int fd );
		Session( const Session & copy );
		~Session( void );
		Session & operator=( const Session & other );

		int 		  getFd( void ) const { return fd; }
		std::string getBuff( void ) const { return buffIn; }
		bool 		toBeWrite( void ) const { return !buffOut.empty(); }
};

std::ostream & operator<<( std::ostream & o, Session & s );
