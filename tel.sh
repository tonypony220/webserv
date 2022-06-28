#!/bin/bash
# /usr/local/nginx
#{ echo "remotecommand 1"; echo "remotecommand 2"; sleep 1; } | telnet localhost 8080
if [[ $1 == a ]]; then
	{
	echo "open localhost 2001";
	sleep 1;
	echo "GET /big_ascii_file.cc HTTP/1.1";
	echo "User-Agent: \"curl/7.54.0sdf\"  \"abd\"";
	echo "Host: \"localhost:2001\"";
	echo "Accept: */*";
	echo ; echo EOF; #sleep 1;
	exit;
	} | tee /dev/tty | telnet #localhost 8080
elif [[ $1 == b ]]
then
	{
	echo "open localhost 8080";
	sleep 1;
	echo "GET / HTTP/1.1";
	echo "User-Agent: \"curl/7.54.0sdf\"  \"abd\"";
	echo "Host: \"localhost:8080\"";
	#echo "Accept: */*";
	echo ; echo; sleep 1;
	} | tee /dev/tty | telnet | grep "HTTP/1.1 " > resp #localhost 8080
elif [[ $1 == c ]]
then
	{
	echo "open localhost 8080";
	sleep 1;
	echo "PUT /upload/t.cpp HTTP/1.1";
	echo "Host: localhost:8080";
	echo "User-Agent: curl/7.54.0";
	echo "Accept: */*";
	#echo "Transfer-Encoding: chunked";
	#echo "Content-Type: application/x-www-form-urlencoded";
	echo "Content-Length: 6";
	echo "Expect: 100-continue";
	echo ;
	sleep 2.11;
	echo "116111";
	#echo "#include "HttpSession.hpp"#include <vector>int main () {	std::vector<HttpSession> v;	HttpSession s(2);//	Session ss(3);	v.push_back(s);	v.push_back(HttpSession(33));	for(std::vector<HttpSession>::iterator it = v.begin(); it != v.end(); ++it) {		std::cout << *it << std::endl;	}}";
	#echo "0"; sleep 1;
} | tee /dev/tty | telnet #localhost 8080
elif [[ $1 == e ]]
then
	{
	echo "open localhost 2001";
	sleep 1;
	echo "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa";
	echo "116111dDDDDDDDDDDDDDDDDDDDDDd";
	#echo "#include "HttpSession.hpp"#include <vector>int main () {	std::vector<HttpSession> v;	HttpSession s(2);//	Session ss(3);	v.push_back(s);	v.push_back(HttpSession(33));	for(std::vector<HttpSession>::iterator it = v.begin(); it != v.end(); ++it) {		std::cout << *it << std::endl;	}}";
	#echo "0"; 
	sleep 1;
} | tee /dev/tty | telnet #localhost 8080

fi
# 		curl -H "Transfer-Encoding: chunked" -d @t.cpp localhost:2001
# 		curl -F "data=@t.cpp" localhost:2001
# this use PUT: 
# 		curl -T t.cpp localhost:2001
