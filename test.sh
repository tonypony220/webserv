#!/bin/bash
# siege --benchmark --internet --concurrent=25 --reps=1  http://localhost:2001/big_ascii_file.cc
#siege -b -i -c 25 -r 1 http://localhost:2001/big_ascii_file.cc


a=$(lsof -c a.out | wc -l | tr -d ' ') # nums opened fds 
# if [ a -eq '0' ]; then 
# 	echo "no $serv process"
# 	exit
# fi
# exit
echo "a=" $a

RED="\033[1;31m"
GREEN='\033[0;32m'
RESET="\033[0;0m"
exit_ok() {
	if [ $? -ne 0 ] ; then
		echo -e $RED"exit code FAILURE"$RESET 
	else 
		echo -e $GREEN "exit code OK" $RESET
	fi
}
code_ok() {
	if [ $code != $1 ] ; then
		echo -e "resp" $code " expected->" $1 $RED" response code FAILURE"  $RESET 
	else 
		echo -e "resp" $code " expected->" $1 $GREEN" response code OK"  $RESET 
	fi
}

check_fds() {
	sleep 1
	cur=$(lsof -c a.out | wc -l | tr -d ' ')
	if [ $cur = $a ]; then
	   echo -e $GREEN "fd's clear" $RESET
	else
	   echo -e $RED "fd's opened! make sure enough time for close before call this check"$RESET 
	fi
	echo -e $BLUE "cur lsof" $cur $RESET
}
##https://manpages.ubuntu.com/manpages/kinetic/en/man1/siege.1.html
echo "expected code 200"
siege -c 5 -r 1 http://localhost:2001/
exit_ok
check_fds

echo "expected code 404"
siege -c 25 -r 1 http://localhost:2001/a
exit_ok
check_fds

echo "expected code 200"
siege -c 25 -r 1 http://localhost:2001/tmp/screenshot.png
exit_ok
check_fds

echo -e "\t\t*** chunked upload check *** "
code=$(curl -s -o /dev/null -w "%{http_code}" -H "Transfer-Encoding: chunked" -d @t.cpp localhost:2001)
code_ok "405"
exit_ok
check_fds


code=$(curl -s -o /dev/null -w "%{http_code}" -T t.cpp localhost:2001/upload/)
code_ok "204"
exit_ok

code=$(curl -s -o /dev/null -w "%{http_code}" -T tmp/screenshot.png localhost:2001/upload/)
code_ok "413"
exit_ok

code=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE localhost:2001/upload/t.cpp)
code_ok "204"
exit_ok

code=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE localhost:2001/upload/t.cpp)
code_ok "404"
exit_ok

code=$(curl -s -o /dev/null -w "%{http_code}" -d @t.cpp localhost:2001/upload/sfile)
code_ok "204"
exit_ok

code=$(curl -s -o /dev/null -w "%{http_code}" -H "Transfer-Encoding: chunked" -d @t.cpp localhost:2001/upload/somefile)
code_ok "204"
exit_ok

code=$(curl -s -o /dev/null -w "%{http_code}"  -H "Transfer-Encoding: chunked" -T t.cpp localhost:2001/upload/)
code_ok "204"
exit_ok
check_fds

 echo "expected code 200"
 siege -c 25 -r 1 http://localhost:2001/cgi/hello_get.py
 exit_ok

echo "\t\t**** checking routing by host name ****\n\n"
code=$(curl -s -o /dev/null -w "%{http_code}" -T big_ascii_file.cc  localhost:2001/upload/)
code_ok "204"
exit_ok

echo "\t\t**** checking routing by host name ****\n\n"
code=$(curl -s -o /dev/null -w "%{http_code}" -T big_ascii_file.cc  hello.org:2001/upload/)
code_ok "413"
exit_ok

check_fds
echo "\n\n ------- put in browser: ---------"
echo "localhost:2001/form.html"

echo "INT on uploading file" 
echo "curl -T big_ascii_file.cc localhost:2001/upload/  -> Ctrl-C" 

## curl -H -s -o /dev/null -w "%{http_code}" "Transfer-Encoding: chunked" -d @t.cpp localhost:2001
## siege -H "Transfer-Encoding: chunked" -c 1 -r 1 http://localhost:2001/

