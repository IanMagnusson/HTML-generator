#!/bin/sh
#
# view_page.cgi
# cgi program to display current state of page
# parses and checks query string and passes it to mwp.cgi
# as QUERY_STRING and as $1
#
# error: no such datafile
#
# code modified by Jack to add security and portability
#
	BASE=$( dirname "$(realpath "$0")" )	# filepath for helpers
	
	# parse QUERY_STRING and make special chars HTML char codes for security
	eval $("$BASE"/qryparse | "$BASE"/clean_qryparse)
	QUERY_STRING=$pagename
	export QUERY_STRING	
		
	DATAFILE="$BASE"/data/"$pagename".tab	# $pagename set by qryparse
	# any datafile?
	if test -f "$DATAFILE"
	then	
		# generate HTML
		"$BASE"/mwp.cgi $pagename
	else
		# No datafile
		echo "Content-type: text/html" ; echo "" # type of output
		
		# HTML header to preserve report back formating
		printf "<!doctype html>\n<html>\n<body>\n<pre><code>"
		echo "Unable to open page."
		echo "No datafile for $pagename ."
		
		# footer
		printf "</code></pre>\n</body>\n</html>\n"
	fi
