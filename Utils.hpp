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

std::vector<std::string> list_dir(std::string path) {
	DIR						 *dir;
	struct dirent			 *ent;
	std::vector<std::string> listing;

	if ((dir = opendir (path.c_str())) != nullptr) {
		while ((ent = readdir(dir)) != nullptr) {
			std::string entry(ent->d_name);
			if (entry == "." || entry == "..")
				continue;
			listing.push_back(entry);
		}
		closedir(dir);
		return listing;
	}
	perror("error list dir");
	return listing;

}

size_t get_file_size(std::string & path) {
	struct stat s;
	size_t sz = 0;
	if (!stat( path.c_str(), &s ) )
		sz = s.st_size;
	return sz;
}

bool valid_dir_path(std::string path) {
	struct stat s;
	if( stat( path.c_str(), &s ) == EXIT_SUCCESS && (s.st_mode & S_IFDIR ))
		return true;
	return false;
}

// recursive search
bool find_file(std::vector<std::string> & filenames,
			   std::string path, // onli dir
			   std::string & result) {
//	std::cout << "\tsearching in dir: " << path << "\n";
	std::vector<std::string> listing = list_dir(path);
	std::vector<std::string> dirs;
	for ( int i=0; i < listing.size(); i++ )
	{
		std::string new_path = path + "/" + listing[i];
		if ( easyfind(filenames, listing[i]) ) {
			result = new_path;
			return true;
		} else {
			if ( valid_dir_path(new_path) )
				dirs.push_back(new_path);
		}

	}
	for ( int i=0; i < dirs.size(); i++ ) {
		if ( find_file(filenames, dirs[i], result) )
			return true;
	}
	return false;
}

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


bool validate_hex( std::string & number ) {
	for ( int i = 0; i < number.size(); i++ ) {
		if ( !isdigit(number[i]) 
		&& ( std::toupper(number[i]) > 70 || std::toupper(number[i]) < 65 ) )
			return false;
	}
	return true;
}
#ifdef CVER
size_t hextoi(const std::string & number) { // case sansitive
	size_t res = 0;
	int j = 0; 
	for (int i = number.size() - 1; i >= 0; i--, j++) {
		int n = number[i] - 48;
		//std::cout << n << ' ';
		if ( n > 10 )
			n -= 7;
		res += pow(16, j) * n;
		//std::cout << pow(16, j) * n << '\n';
	}
	return res;
}
#else
size_t hextoi(const std::string & number) {
	unsigned int x;   
    std::stringstream ss;
    ss << std::hex << number;
    ss >> x;
	return x;
}
#endif
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
int unpack_dequtes(std::string & buffer) {
	std::string::size_type pos = buffer.find("\"");
	while ( pos != std::string::npos ) {
		std::cout << "unpacking: " << buffer << '\n';
		buffer.erase(pos, 1);	
		pos = buffer.find("\"", pos);
		if ( pos == std::string::npos )
			return EXIT_FAILURE;
		buffer.erase(pos, 1);
		pos = buffer.find("\"");
	}
	return EXIT_SUCCESS;
}

void ltrim(std::string & s) {
	s.erase(s.begin(),
			std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void rtrim(std::string & s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
						 std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void trim(std::string & s) {
	ltrim(s);
	rtrim(s);
}

void to_lower(std::string & str)
{
//	int len = ;
//	for (int i=0; i < static_cast<int>(str.length()); ++i) {
	for (int i=0; i < str.length(); ++i) {
		str[i] = std::tolower(str[i]);
	}
}


void replace(std::string & str, std::string const & s1, std::string const & s2)
{
	std::size_t found = str.find( s1 );
	while ( found != std::string::npos ) {
		/* std::cout << found << std::endl; */
		str.erase( found, s1.length() );
		str.insert( found, s2 );
		found = str.find( s1 );
	}
}

bool isDigits(const std::string & str) {
	//https://stackoverflow.com/questions/8888748/how-to-check-if-given-c-string-or-char-contains-only-digits
	return str.find_first_not_of("0123456789") == std::string::npos;
}

bool isDigitsAndOws(const std::string & str) {
	return str.find_first_not_of("0123456789 \t") == std::string::npos;
}
