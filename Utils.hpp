#pragma once
#include <map>
#include <vector>
#include <algorithm>

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
