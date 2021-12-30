#include <iostream>
#pragma_once

class One {
	private:
		int	 fd;
		std::string buffIn;
		std::string buffOut;
	public:
		One( int fd );
		One( const One & copy );
		~One( void );
		One & operator=( const One & other );

		int 		  getFd( void )   const { return fd; }
		std::string getBuff( void )   const { return buffIn; }
		bool 		toBeWrite( void ) const { return !buffOut.empty(); }
};

std::ostream & operator<<( std::ostream & o, One & s );
