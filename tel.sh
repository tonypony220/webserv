#!/bin/bash
#{ echo "remotecommand 1"; echo "remotecommand 2"; sleep 1; } | telnet localhost 8080
{
echo "open localhost 8080";
sleep 1;
echo "POST /re HTTP/1.1";
echo "User-Agent: curl/7.54.0";
echo "Host: localhost:8080";
#echo "Accept: */*"; 
echo ; echo; sleep 1;
} | tee /dev/tty | telnet #localhost 8080

