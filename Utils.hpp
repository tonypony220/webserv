#pragma once
#include <sstream>
#include <iostream>
#include <map>
#include <cmath>
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#define ERROR 	  		-1
#define SUCCESS 		 0
#define END 			 1
#define RED 	"\033[1;31m"
#define GREEN   "\033[0;32m"
#define YELLOW	"\033[1;33m"
#define BLUE	"\033[1;34m"
#define PURPLE  "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0;0m"
#define BOLD    "\033[;1m"
#define REVERSE "\033[;7m"
#define VERBOSE


std::string cgi_extensions_supported[] = {"cgi", "py", "sh"};
std::string HttpMethods[] = {"GET", "POST", "PUT", "CONNECT",
							 "DELETE", "OPTIONS", "TRACE"};
std::string HttpMethodsImplemented[] = {"GET", "POST", "DELETE", "PUT" };
//bool find_file(std::vector<std::string> & filenames,
//			   std::string path,
//			   std::string & result) {
//	std::cout << "\tsearching in dir: " << path << "\n";
//	std::vector<std::string> listing = list_dir(path);
//	std::vector<std::string> dirs;
//	for ( int i=0; i < listing.size(); i++ )
//	{
//		if (listing[i] == "." || listing[i] == "..") {
//			listing.erase(listing.begin() + i);
//			i--;
//			continue;
//		}
//		std::string new_path = path + "/" + listing[i];
//		if ( easyfind(filenames, listing[i]) ) {
//			result = new_path;
//			return true;
//		} else {
//			if ( !valid_dir_path(new_path) ) {
//				listing.erase(listing.begin() + i);
//				i--;
//			}
////				dirs.push_back(new_path);
//		}
//
//	}
//	for ( int i=0; i < listing.size(); i++ ) {
//		if ( find_file(filenames, path + "/" + listing[i], result) )
//			return true;
//	}
//	return false;
//}

template <typename T>
void p(T a) {
//	std::cout << "data: ";
	for (typename T::iterator i = a.begin(); i != a.end(); i++)  {
			std::cout << "(" << *i << ") ";
	}
	std::cout << std::endl;
}

void handler(int s) {
	printf("Caught SIGPIPE\n");
}


template <class T> 
void log(const T & t) { std::cout << t << std::endl; }

template <class T, class T2> 
void log(const T & t, const T2 & t2) { std::cout << t  << t2 << std::endl; }

template <class T, class T2, class T3> 
void log(const T & t, const T2 & t2, const T3 & t3) {
	std::cout << t << ' ' << t2 <<' ' << t3 << std::endl;
}

template <class T, class T2, class T3, class T4> 
void log(const T & t, const T2 & t2, const T3 & t3, const T4 & t4) {
	std::cout << t << ' ' << t2 <<' ' << t3 << ' ' << t4 << std::endl;
}


template <class T>
std::string itoa(T & i) {
	std::stringstream out;
	out << i;
	return out.str();
		//s = out.str();
}

template <class T1, class T2>
void construct(T1* p, const T2& value) {
	new (p) T1(value);
   // new (reinterpret_cast<void*>(p)) T1 ( value );
}
template <class T, class Val>
bool easyfind (T & iterable, Val val, size_t * idx = nullptr) {
	typename T::iterator found = find( iterable.begin(), iterable.end(), val );
	if (found != iterable.end()) {
		if (idx)
			*idx = found - iterable.begin();
		return true;
	}
	return false;
}
std::vector<std::string> list_dir(std::string path) ;

size_t get_file_size(std::string & path) ;

bool valid_dir_path(std::string path) ;

// recursive search
bool find_file(std::vector<std::string> & filenames,
			   std::string path, // onli dir
			   std::string & result) ;

bool validate_hex( std::string & number ) ;
size_t hextoi(const std::string & number) ;
int unpack_dequtes(std::string & buffer) ;

void ltrim(std::string & s) ;

void rtrim(std::string & s) ;

void trim(std::string & s) ;

void to_lower(std::string & str);

void replace(std::string & str, std::string const & s1, std::string const & s2);

bool isDigits(const std::string & str) ;

bool isDigitsAndOws(const std::string & str) ;
