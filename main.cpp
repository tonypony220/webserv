#include "socket.hpp" 

int main (){ 
	Socket s;	
	std::cout << s.getFd();
	if (s.getFd() < 0)
		std::cout << s.getBuff() << std::endl;
}
