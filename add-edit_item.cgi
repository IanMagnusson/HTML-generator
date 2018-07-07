#!/bin/sh
#
# add_item.cgi
# this script is called from ewp.html for the Add function
# it takes the specified items and creates a new record on the
# end of the named file in the data directory
#
# modified by Jack to update existing items
# updated item will remain in its orignal place in table of items
#
# error: incomplete/invalid input
#
# script uses qrypase and clean_qryparse to remove special chars for security
#
	
	retrieve_clean_and_check_input()
	{
	# parse QUERY_STRING and make special chars HTML char codes for security
	eval $("$BASE"/qryparse | "$BASE"/clean_qryparse)
	
	# if any var empty
	if ! (test -n "$title" && test -n "$titlecolor" && \
		test -n "$descrip" && test -n "$url" && test -n "$pagename")
	then
		echo "Incomplete data."
		echo "Please press the back button and try again."
		exit
	fi
	
	# is titlecolor valid color (Hex triplet or name in list)?
	if ! (echo "$titlecolor" | grep -q -E '^#[0-9a-fA-F]{6}$' ||
		grep -w -q -i "${titlecolor}" "$BASE"/colornames)
	then
		echo "$titlecolor is not a valid color."
		echo "Valid color values are HEX triplets or name below:"
		cat "$BASE"/colornames
		exit
	fi

	# Build rest of vars	
	DATAFILE="$pagename".tab
	NEWITEM=$(printf "title=%s|tcolor=%s|desc=%s|url=%s" \
		"$title" "$titlecolor" "$descrip" "$url")
	}

	check_data_dir()
	{	
	if test ! -d data		# if no data directory
	then
		mkdir data		# try to make it
		if test $? -ne 0	# and check for errors
		then
			echo "Cannot create data directory."
			exit
		fi
	fi
	}

	add_update_item()
	{	
	cd data
	# item already exists in data file? (also fails if no DATAFILE)
	if grep -q "^title=${title}|" "$DATAFILE" 2>/dev/null
	then
		# prepare newitem for sed s by escaping &s
		SUBSNEWITEM=$(echo "$NEWITEM" | sed s/'&'/'\\&'/g)
		#update item
		sed "s<^title=${title}|.*<${SUBSNEWITEM}<" \
			"$DATAFILE" > "${DATAFILE}.tmp"
		mv "${DATAFILE}.tmp" "$DATAFILE"
	else	
		#append new item
		echo "$NEWITEM" >> "$DATAFILE"
	fi
	
	#report success or error
	if test $? -eq 0
	then
		echo "Update of $title was successful."
		echo "Press the Back button now."
	else
		echo "Unable to update file $DATAFILE ."
		echo "Please report to page admin."
	fi
	}

	# Main Script

	BASE=$( dirname "$(realpath "$0")" ) # get filepath for helpers
	
	echo "Content-type: text/HTML"	# tell browser type of output
	echo ""				# end of header	

	# HTML header to preserve report back formating
	printf "<!doctype html>\n<html>\n<body>\n<pre><code>"	

	retrieve_clean_and_check_input

	check_data_dir
	
	add_update_item
	
	# footer
	printf "</code></pre>\n</body>\n</html>\n"
