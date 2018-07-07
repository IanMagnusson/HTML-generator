#!/bin/sh
# demo1.cgi
# demonstrates how a URL with a trailing ?string passes the
# string as $QUERY_STRING and as $1

	printf "Content-type: text/plain\n\n"
	echo "My name is $0"
	echo "QUERY_STRING has the value: " $QUERY_STRING
	echo The number of args is $#
	echo The args are $*

