#!/bin/bash

#if [ -z grep $address /etc/passwd ]
if !(grep hello /etc/hosts) ; then
	sudo echo "127.0.0.1       hello.org" >> /etc/hosts
	echo "hello.org added"
fi
