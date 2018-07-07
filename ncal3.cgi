#!/bin/sh
#	a cgi for ncal2 that takes QUERY_STRING input
#	expects a single month (alpha or numeric)	
#	code by Jack

	BASE=$( dirname "$(realpath "$0")" )

	echo "Content-type: text/plain"
	echo ""

	#limit input to alphanumeric
	QUERY_STRING=$(echo $QUERY_STRING | sed s/[^a-zA-Z0-9]//g)

	#bad input?
	if ! QUERY_STRING=$("$BASE"/name2num $QUERY_STRING)
	then
		echo query_string must be a month
		exit 1
	fi
	
	#month already past?	
	set $(date)
	MON=$("$BASE"/name2num $2) ; YEAR=$6
	if [ $QUERY_STRING -lt $MON ]
	then
		#yes, print month for next year
		YEAR=$(expr $YEAR + 1)
		"$BASE"/ncal2 $QUERY_STRING $YEAR
	else
		#no, print month for this year
		"$BASE"/ncal2 $QUERY_STRING
	fi
