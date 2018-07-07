#!/bin/sh
#	mwp.cgi
#	code by Jack
#		purpose: builds HTML from datafile
#		input: QUERY_STRING is datafile basename, '|' delim
#		error: if no such datafile
#
	
	generate_page()
	{
	#header with $PAGENAME standin replaced by SUBSPAGENAME var string
	SUBSPAGENAME=$(echo "$PAGENAME" | sed s/'&'/'\\&'/g)	# prep for sed
	cat "$BASE"/header | sed s/'$PAGENAME'/"$SUBSPAGENAME"/g
	
	#table rows from | delimited data
	"$BASE"/fl format '-d|' "data/$PAGEDATA"

	#footer
	cat "$BASE"/footer
	}

	# Main Script

	BASE=$( dirname "$(realpath "$0")" ) # filepath for helpers
	
	echo "Content-type: text/html"
	echo ""	

	PAGENAME=$QUERY_STRING
	PAGEDATA="${PAGENAME}.tab"
	
	#if there is a data file
	if test -f ""$BASE"/data/$PAGEDATA"
	then
		generate_page
	else
		echo ERROR NO SUCH DATA FILE "$PAGEDATA"
	fi
