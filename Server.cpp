#include "Server.hpp"

// WARNING this must not be changed as it now externed with size!!!
std::string cgi_extensions_supported[] = {"cgi", "py", "sh"};
std::string HttpMethods[] = {"GET", "POST", "PUT", "CONNECT",
							 "DELETE", "OPTIONS", "TRACE"};
std::string HttpMethodsImplemented[] = {"GET", "POST", "DELETE", "PUT" };
// WARNING this must not be changed as it now externed with size!!!


void clear_path(std::string & path) {
	size_t pos = path.find("//");
	if (pos != std::string::npos)
		path.erase(pos, 1);
}

//bool vars_correct(std::vector<std::string> & vars, std::string *refs[]) {
//	std::set<std::string> r(std::begin(refs), std::end(refs));
//	size_t sz(r.size());
//	r.insert(vars.begin(), vars.end());
//	return sz == r.size();
//}
bool vars_correct(std::vector<std::string> & vars,
				  std::string * begin,
				  std::string * end)
{
	std::set<std::string> r(begin, end);
	size_t sz(r.size());
	r.insert(vars.begin(), vars.end());
	return sz == r.size();
}
//std::map<std::string, std::string> execs = {std::make_pair(".py", "python3")};
	void Location::display(std::ostream & o) {
		o << "\tLocations: " << "\n";
		o << "\t\troute: " << route << "\n";
		if (root.size())
		o << "\t\troot: " << root << "\n";
		if (redirect_uri.size())
		o << "\t\tredirect: " << redirect_uri << "\n";
		o << "\t\tautoindex: " << dir_listing << "\n";
		if (cgi_extensions.size()) {
			o << "\t\tcgi's: ";
			p(cgi_extensions);
			o << "\n";
		}
		o << "\t\tmethods: ";
		p(allowed_methods);
		o << "\n";
	}
	void Location::validate(std::stringstream & err) {
		if (root.size() && !valid_dir_path(root)) {
			err << "bad root: " << root << " " << strerror(errno);
//			return;
		}
		if (!vars_correct(allowed_methods,
						  std::begin(HttpMethodsImplemented),
						  std::end(HttpMethodsImplemented))) {
			err << "bad methods "<< "\n";
//			return;
		}
		if (!vars_correct(cgi_extensions,
						  std::begin(cgi_extensions_supported),
						  std::end(cgi_extensions_supported))) {
			err << "bad cgi: " << "\n";
//			return;
		}
	}

	bool Location::method_allowed(std::string & method) {
		if (allowed_methods.empty())
			return true;
		return find(allowed_methods.begin(),
			  		allowed_methods.end(),
			        	 method) != allowed_methods.end();
	}
	void Location::enable_cgi(const std::string & file_extension) {
		cgi_extensions.push_back(file_extension);
	}
	bool Location::is_cgi_file_type(std::string & file_type) {
//		log(file_type, " cgi extensions:  ");
//		p(cgi_extensions);
		std::vector<std::string>::iterator found = find(
				cgi_extensions.begin(),
				cgi_extensions.end(),
				file_type);
		if ( found != cgi_extensions.end())
			return true;
		return false;

	}
	void Location::clear() {
		route.clear();
		root.clear();
		redirect_uri.clear();
		dir_listing = false;
		cgi_extensions.clear();
		allowed_methods.clear();
	}

std::ostream & operator<<( std::ostream & o, Location & s ) {
	s.display(o);
	return ( o );
}


	void Server_config::display(std::ostream & o) {
		o << "Server Config: " << "\n";
		o << "\troot: " << root << "\n";
		o << "\terror pages path: " << root << "\n";
		o << "\tmax size: " << max_size << "\n";
		o << "\tenable sessions: " << enable_session << "\n";
		o << "\tserver names: ";
		p(server_names);
		o << "\n";
		o << "\tports: ";
		p(ports);
		o << "\n";
		o << "\tLocations:";
		p(locs);
	}

	//        location     path
//	std::map<std::string, std::string> locs;
	Location * Server_config::route_target_path(std::string & target) {
		// location/this/abc -> root/abc
		size_t max = 0;
		size_t idx = 0;
		for (size_t i=0; i<locs.size(); i++){
			if (target.find(locs[i].route) != std::string::npos) {
				if (locs[i].route.size() > max) {
					max = locs[i].route.size();
					idx = i;
				}
			}
		}
		if (max) {
			target.erase(0, locs[idx].route.size());
			std::string r = locs[idx].root.size() ? locs[idx].root : root;
			target.insert(target.begin(), r.begin(), r.end());
			return &locs[idx];
		}
		return nullptr;

	}
	bool Server_config::server_name_in_config_names(std::string & name) {
		return easyfind(server_names, name);
	}

	void Server_config::add_port(int port) {
		ports.push_back(port);
	}
	void Server_config::clear() {
		root.clear();  // required field
		server_names.clear();
		error_pages_path.clear();
		ports.clear();
		locs.clear();
		max_size = 0;
		enable_session = false;
	}

std::ostream & operator<<( std::ostream & o, Server_config & s ) {
	s.display(o);
	return ( o );
}



	bool Server::configs_intersects(Server_config & new_config) {
		for (int i=0; i < configs.size(); i++) {
			Server_config & conf = configs[i];
			if (intersects(conf.ports, new_config.ports)
			&& (intersects(conf.server_names, new_config.server_names))) {
//			for (int j=0; j < conf.ports.size(); j++) {
//				if (conf.ports[j] == port && server_name == conf.server_name)
					return true;
			}
		}
		return false;
	}

	void Server::display_configs_addresses() { // debuging purpose
		for (int i = 0; i < configs.size(); i++) {
			std::cout << "config* = " << &configs[i] << "\n";
			std::cout << configs[i];
		}

		std::map<int, std::vector<Server_config*> >::iterator it = mapping.begin();
		for (; it != mapping.end(); it++) {
			for (int i = 0; i < it->second.size(); i++)
				std::cout << "port config = " << it->second[i] << "\n";
		}
	}

	Server_config * Server::get_default_config() { return &configs[0]; }

	Server_config * Server::match_config(std::string name, int port) {
//		display_configs_addresses();
		log("matching config for ", name, " ", port);
		if (!port)
			return &configs[0];
		std::vector<Server_config*> conf = mapping[port];
		int i = conf.size();
//		if (!i)
//			std::cerr << "<<<<<<<<< no config?" << conf[i] << "\n";
		i--;
		for (; i > 0; i--) {
			if (conf[i]->server_name_in_config_names(name))
				return conf[i];
		}
		std::cout << "<<<<<<<<< saved config" << conf[i] << "\n";
		return conf[i]; // default
	}

	void Server::validate_config(Server_config & config) {
		if (ok() && config.root.empty())
			err << "root required";
		if (ok() && config.ports.empty())
			err << "listen ports required";
		if (ok() && config.locs.empty())
			err << "no location provided";
		if (ok() && !valid_dir_path(config.root))
			err << "bad root: " << config.root << " " << strerror(errno);
		if (ok()
		&& config.error_pages_path.size()
		&& !valid_dir_path(config.error_pages_path))
			err << "bad err pages path: " << config.root << " "
			<< strerror(errno);
		for (int i = 0; i < config.locs.size() && ok(); i++) {
			config.locs[i].validate(err);
		}
	}

	bool Server::ok() { return !err.rdbuf()->in_avail(); }

	int Server::error() {
		std::cerr << RED << "config error: " << err.rdbuf() << RESET"\n";
		return ERROR;
	}

	int Server::add_config(Server_config & config) {
		validate_config(config);
		if (!ok())
			return error();
		if ( configs_intersects(config) ) {
//			err << RED"config error: server names conflict\n"RESET;
			return error();
		}
		configs.push_back(config);
		// to map pointer on objs in this vector
		Server_config * conf = &configs.back();
//		std::cout << "<<<<<<<< parser config" << &config << "\n";
//		std::cout << "<<<<<<<< serv   config" << conf << "\n";
		for ( int i=0; i<config.ports.size(); i++ ) {
			int port = config.ports[i];
			if ( ports.insert(port).second )
//			if ( ports.find(port) == ports.end() )
				sockets.push_back( SocketTCP(port) );
			mapping[port].push_back(conf);
		}
		return SUCCESS;
	}

	int Server::create(std::vector<Server_config> & parser_configs) {
		configs.reserve(parser_configs.size());
		for ( size_t i = 0; i < parser_configs.size(); i++ ) {
			if (add_config(parser_configs[i]))
				return EXIT_FAILURE;
		}
		display_configs_addresses();
		return EXIT_SUCCESS;
	}
	std::string Server::generate_session_id() {
		std::stringstream ss;

		srand(time(NULL) + rand());
//		for(int i = 0;i < 64;i++)
//		{
//			int j = rand();
////			while(j < 32)
////				j = rand() % 127;
//			ss << std::hex << j;
////			ss << char(j);
//		}
		int j = rand();
//			while(j < 32)
//				j = rand() % 127;
		ss << std::hex << j;
		return ss.str();
	}

//	size_t parse_session_id(std::string & s) {
//
//	}

	void Server::set_server_name(std::string & name) { app_name = name; }

Server::Server() :app_name("pony server") {}
Server::~Server() {}
	//Server( const Server & copy ) { *this = copy; }
	//Server & operator=( const Server & other ) {}
