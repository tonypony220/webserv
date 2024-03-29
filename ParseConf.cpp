#include "ParseConf.hpp"
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

bool commented_line(std::string & line) {
	size_t i = line.find("#");
	if (i != std::string::npos) {
		for (; i >= 0; i--) {
			if (!isspace(line[i]))
				return true;
		}
	}
	return false;
}
void log_err(const std::string & msg) {
	std::cerr << RED << msg << RESET"\n";
}

//https://www.tutorialspoint.com/python/python_cgi_programming.htm

ParserConfig::ParserConfig(const std::string & filename) {
		file.open(filename);
	}

	void ParserConfig::parse_location_block() {
//		std::cout << "\t\t* cur locatino token: " << token << "\n";
//		std::string route = token;
		while (iss) {
//			std::cout << "- - -cur locatino token: " << token << "\n";
			if ( token == "}") {
//				log("- - location parsed");
				unset_state(LOCATION);
				unset_state(LOCATION_BLOCK_OPENED);
				config.locs.push_back(loc);
				loc.clear();
				return;
			}
			else if (token == "{") {
				set_state(LOCATION_BLOCK_OPENED);
			}
			else if (!(state & LOCATION_BLOCK_OPENED)) {
				err_msg << "bad location syntax: {";
//				return EXIT_FAILURE;
			}
//			else if (token == "autoindex") {
//				loc.dir_listing = true;
//			}
			else if ( token == "allowed_methods")
				parse_seq(loc.allowed_methods, iss);
			else if (token == "root") {
				iss >> loc.root;
				if (loc.root.back() != '/')
					loc.root += "/";
			}
			else if (token == "redirect")
				iss >> loc.redirect_uri;
			else if (token == "index")
				parse_seq(loc.filenames, iss);
			else if (token == "cgi")
				parse_seq(loc.cgi_extensions, iss);
			else if (token == "autoindex") {
				iss >> token;
				if (token == "on")
					loc.dir_listing = true;
				else if (token == "off")
					loc.dir_listing = false;
				else {
					err_msg << "unrecognized token in location block: '" << token << "'\n";
				}
			}
			else
				err_msg << "unrecognized token in location block: '" << token << "'\n";
			iss >> token;
		}
	}


	void ParserConfig::parse_server_block() {
		while (iss) {
//			std::cout << "cur token: " << token << '\n';
			if (state & LOCATION) {
				parse_location_block();
			}
			else if (token == "{") {
				set_state(SERVER_BLOCK_OPENED);
			}
			else if (!(state & SERVER_BLOCK_OPENED)) {
				err_msg << "bad server syntax: {";
//				return EXIT_FAILURE;
			}
			else if (token == "}") {
//				log("- - server parsed");
				unset_state(SERVER_BLOCK_OPENED);
				unset_state(SERVER);
				configs.push_back(config);
			}
			else if (token == "location") {
//				iss >> token;
				iss >> loc.route;
				set_state(LOCATION);
			}
			else if (token == "server_name") {
				parse_seq(config.server_names, iss);
			}
			else if (token == "listen") {
				parse_seq(config.ports, iss);
			}
			else if (token == "client_max_body_size") {
				iss >> token;
				config.max_size = atoi(token.c_str());
			}
			else if (token == "root") {
				iss >> config.root;
				if (config.root.back() != '/')
					config.root += "/";
			}
			else if (token == "enable_session")
				config.enable_session = true;
			else if (token == "error_page")
				iss >> config.error_pages_path;
			else {
				err_msg << "unrecognized token in server block: '" << token << "'\n";
//				return EXIT_FAILURE;
			}
			iss >> token;
		}
//		return EXIT_SUCCESS;
	}

	void ParserConfig::unset_state(int macro_state) { state &= ~macro_state; }
	void ParserConfig::set_state(int macro_state) { 	state |= macro_state; }

	int ParserConfig::parse_file() {
		if (!file)
			err_msg << "error open file: " << strerror(errno) << "\n";
		std::string line;
		while (std::getline(file, line) && !err_msg.rdbuf()->in_avail()) {
//			std::istringstream iss(line);
			if (commented_line(line))
				continue;
//			std::cout << BLUE" - parsing line: " <<  line << RESET"\n";
			iss.clear();
			iss.str(line);
			iss >> token;

			if (token == "server") {
				iss >> token;
				set_state(SERVER);
				config.clear();
			}
			if (state & SERVER) {
				parse_server_block();
//				int ret = parse_server_block();
//				if ( ret == EXIT_FAILURE )
//					return EXIT_FAILURE;
			}
		}
		if (configs.empty())
			err_msg << "no servers to be run";

		file.close();
		if ( err_msg.rdbuf()->in_avail() ) {
			std::cerr << RED << "config error: " << err_msg.rdbuf() << RESET"\n";
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
