#pragma once
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
# define RED 	 "\033[1;31m"
# define GREEN   "\033[0;32m"
# define YELLOW	 "\033[1;33m"
# define BLUE	 "\033[1;34m"
# define PURPLE  "\033[1;35m"
# define CYAN    "\033[1;36m"
# define RESET   "\033[0;0m"
# define BOLD    "\033[;1m"
# define REVERSE "\033[;7m"
#define VERBOSE

template <class T> 
void log(const T & t) { std::cout << t << std::endl; }

template <class T, class T2> 
void log(const T & t, const T2 & t2) { std::cout << t  << t2 << std::endl; }

template <class T, class T2, class T3> 
void log(const T & t, const T2 & t2, const T3 & t3) { std::cout << t << ' ' << t2 <<' ' << t3 << std::endl; }

template <class T, class T2, class T3, class T4> 
void log(const T & t, const T2 & t2, const T3 & t3, const T4 & t4) { std::cout << t << ' ' << t2 <<' ' << t3 << ' ' << t4 << std::endl; }

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
