#pragma once
//#include "Session.hpp"
#include "SocketTCP.hpp"
//#include "Request.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <set>
#include <sys/stat.h>

extern std::string cgi_extensions_supported[3];
extern std::string HttpMethods[7];
extern std::string HttpMethodsImplemented[4];

void clear_path(std::string & path) ;

//bool vars_correct(std::vector<std::string> & vars, std::string *refs[]) {
//	std::set<std::string> r(std::begin(refs), std::end(refs));
//	size_t sz(r.size());
//	r.insert(vars.begin(), vars.end());
//	return sz == r.size();
//}
bool vars_correct(std::vector<std::string> & vars,
				  std::string * begin,
				  std::string * end);

//std::map<std::string, std::string> execs = {std::make_pair(".py", "python3")};
struct Location {
	std::string				 route;
	std::string 			 root;
	std::string 		     redirect_uri;
	bool 					 dir_listing;
	std::vector<std::string> cgi_extensions;
	std::vector<std::string> filenames;
	std::vector<std::string> allowed_methods;

	void display(std::ostream & o) ;
	void validate(std::stringstream & err) ;
	bool method_allowed(std::string & method) ;
	void enable_cgi(const std::string & file_extension) ;
	bool is_cgi_file_type(std::string & file_type) ;
	void clear() ;
};

std::ostream & operator<<( std::ostream & o, Location & s ) ;

class Server_config {
public:
	std::string 				root;  // required field
	std::vector<std::string> 	server_names;
	std::string 				error_pages_path;
	std::vector<int> 			ports;
	std::vector<Location>		locs;
	size_t						max_size;
	bool						enable_session;
	std::map<std::string, std::string> execs; // ?

	void display(std::ostream & o) ;
	//        location     path
//	std::map<std::string, std::string> locs;
	Location * route_target_path(std::string & target) ;
	bool server_name_in_config_names(std::string & name) ;
	void add_port(int port) ;
	void clear() ;
};

std::ostream & operator<<( std::ostream & o, Server_config & s ) ;

template <class T>
bool intersects(std::vector<T> a, std::vector<T> b ) {
	std::set<T> s;
	s.insert(a.begin(), a.end());
	s.insert(b.begin(), b.end());
	if (s.size() != a.size() + b.size())
		return true;
	return false;
}

class Server {
public:
	std::vector<Server_config> 					 configs;
	std::vector<SocketTCP> 						 sockets;
	std::set<int> 								 ports;
	std::string 								 app_name;
	std::map<int, std::vector<Server_config*> >  mapping;
//	std::unordered_map<int, std::vector<Server_config*> >  mp;

	std::stringstream err;

	bool configs_intersects(Server_config & new_config) ;

	void display_configs_addresses() ; // debuging purpose

	Server_config * get_default_config() ;

	Server_config * match_config(std::string name, int port) ;
	void validate_config(Server_config & config) ;
	bool ok() ;

	int error() ;
	int add_config(Server_config & config) ;

	int create(std::vector<Server_config> & parser_configs) ;
	std::string generate_session_id() ;

	void set_server_name(std::string & name) ;

	Server();
	~Server();
	//Server( const Server & copy ) { *this = copy; }
	//Server & operator=( const Server & other ) {}
};
