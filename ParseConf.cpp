#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include "Utils.hpp"
#include "Server.hpp"
#define SERVER 1
#define LOCATION 2

//bool last_token(std::string & token) {
//	std::string::size_type pos = token.find(";");
//	if (pos == std::string::npos)
//		return false;
//
//	while (pos != std::string::npos) {
//		token.erase(pos, 1);
//		pos = token.find(";");
//	}
//	return true;
//}

int parse_seq(std::vector<std::string> & data, std::istringstream & iss) {
	std::string token;
//	bool end = false;
//	while (!end && iss) {
	while (iss) {
		iss >> token;
//		if (last_token(token)) {
//			end = true;
//			data.push_back(token);
//		}
		std::cout << BLUE<< token << "  "RESET;
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

	ParserConf(const std::string & filename) {
		file.open(filename);
	}
	void parse_location_block() {
		iss >> token;
		std::string route = token;
		while (iss && token != "}") {
			if (token == "autoindex") {}
			else if ( token == "allowed_methods")
				parse_seq(data, iss);
			else if (token == "root") {}
			else if (token == "enable_cgi")
				parse_seq(data, iss);
			else
				std::cerr << "error";
		}
	}

	int parse_server_block() {
		while (iss) {
			std::cout << token << '\n';
			if (state & LOCATION)
				parse_location_block();
			if (token == "}") {
				configs.push_back(config);
			}
			else if (token == "location") {
				set_state(LOCATION);
			}
			if (token == "server_name") {
				int ret = parse_seq(config.server_names, iss);
			}
			else if (token == "listen") {
				int ret = parse_seq(config.ports, iss);
			}
			else if (token == "client_max_body_size") {
				iss >> token;
				config.max_size = atoi(token);
			}
			else if (token == "root")
				iss >> config.root;
			else if (token == "enable_session")
				config.enable_session = true;
			else if (token == "error_page")
				iss >> config.error_pages_path;
			else {
				std::err << "unrecognized token: " << token;
				return EXIT_FAILURE;
			}
			iss >> token;
		}
		return EXIT_SUCCESS;
	}

	void unset_state(int macro_state) { state &= ~macro_state; }
	void set_state(int macro_state) { 	state |= macro_state; }

	void parse_file() {
		if (!file)
			std::cout << "error\n";
		std::string line;
		while (std::getline(file, line)) {
//			std::istringstream iss(line);
			iss.clear();
			iss.str(line);
			iss >> token;
			if (token == "server") {
				set_state(SERVER);
				config.clear();
			}
			if (state & SERVER)
				parse_server_block();

//			while (iss >> token) {
//				std::cout << token << "\n";
//			}
//			if (!(iss)) {
//				break;
//			} // error

			// process pair (a,b)
		}
	}

};

int main() {
	ParserConf parser("file");
	parser.parse_file();
}