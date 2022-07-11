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
	   echo -e $RED "fd's opened!"$RESET 
	fi
	echo -e $BLUE "cur lsof" $cur $RESET
}
##https://manpages.ubuntu.com/manpages/kinetic/en/man1/siege.1.html
#echo "expected code 200"
#siege -c 25 -r 1 http://localhost:2001/
#exit_ok
#check_fds
#
#echo "expected code 404"
#siege -c 25 -r 1 http://localhost:2001/a
#exit_ok
#check_fds
#
#echo "expected code 200"
#siege -c 25 -r 1 http://localhost:2001/tmp/screenshot.png
#exit_ok
#check_fds
#
#echo -e "\t\t*** chunked upload check *** "
#code=$(curl -s -o /dev/null -w "%{http_code}" -H "Transfer-Encoding: chunked" -d @t.cpp localhost:2001)
#code_ok "405"
#exit_ok
#check_fds
#curl -T t.cpp localhost:2001
code=$()
code_ok "204"

code=$(curl -s -o /dev/null -w "%{http_code}" -H "Transfer-Encoding: chunked" -d @t.cpp localhost:2001/upload/somefile)
code_ok "204"

## curl -H -s -o /dev/null -w "%{http_code}" "Transfer-Encoding: chunked" -d @t.cpp localhost:2001
## siege -H "Transfer-Encoding: chunked" -c 1 -r 1 http://localhost:2001/

