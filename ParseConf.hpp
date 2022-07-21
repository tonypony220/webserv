#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
//#include "Utils.hpp"
#include "Server.hpp"
#define SERVER 1
#define LOCATION 2
#define SERVER_BLOCK_OPENED 4
#define LOCATION_BLOCK_OPENED 8

bool commented_line(std::string & line) ;
void log_err(const std::string & msg) ;

template <class T>
int parse_seq(std::vector<T> & data, std::istringstream & iss) {
	T token;
	while (iss >> token) {
		data.push_back(token);
	}
	return 1;
}

//https://www.tutorialspoint.com/python/python_cgi_programming.htm

class ParserConfig {
public:
	int state;
	std::ifstream file;
	std::istringstream iss;
	std::string token;
	std::vector<Server_config> configs;
	Server_config config;
	Location loc;

	std::stringstream err_msg;

	ParserConfig(const std::string & filename) ;

	void parse_location_block();
	void parse_server_block() ;

	void unset_state(int macro_state);
	void set_state(int macro_state) ;
	int parse_file() ;
};
