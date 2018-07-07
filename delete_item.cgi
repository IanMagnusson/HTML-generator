#!/bin/sh
#
# delete_item.cgi
# this script is called from ewp.html for the delete function
# it deletes the specified item if it already exists in the given page
# if delted item was last in page, deletes page
# errors: no such page, no such item, cannot create data dir, incomplete input
#
# script uses qrypase and clean_qryparse to remove special chars for security
#
# code modified from add-edit_item.cgi by Jack
#	
	retrieve_clean_and_check_input()
	{
	# parse QUERY_STRING and make special chars HMTL char codes for security
	eval $("$BASE"/qryparse | "$BASE"/clean_qryparse)

	# empty vars?
	if ! (test -n "$pagename" && test -n "$title")	
	then
		echo "Incomplete data."
		echo "Please press the back button and try again."
		exit
	fi
	
	# build rest of vars
	DATAFILE="$pagename".tab		# $pagename set by qryparse
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
	
	delete_item()
	{
	cd data

	# datafile exists?	
	if test -f "$DATAFILE"
	then
		# item in datafile?
		if grep -q "title=${title}|" "$DATAFILE"
		then
			# yes, delete item (grep -v doesn't report match)
			grep -v "title=${title}|" \
				 "$DATAFILE" > "${DATAFILE}.tmp"
			# anything left?
			if test ! -s "${DATAFILE}.tmp"
			then
				#no, clean up empty files.
				rm "${DATAFILE}.tmp" "$DATAFILE"
				echo "Last item deleted."
				echo "Page $pagename has been removed."
			else
				#yes, replace orig with tmp
				mv "${DATAFILE}.tmp" "$DATAFILE"
				echo "Deletion of $title was successful."
				echo "Press the back button now."
			fi
		else
			# item not in datafile, error
			echo "Unable to delete item $title."
			echo "No such item in $DATAFILE ."
		fi
	else
		# no datafile
		echo "Unable to delete item $title."
		echo "No datafile for $pagename ."
	fi
	}
	
	# Main Script
	
	BASE=$( dirname "$(realpath "$0")" ) # filepath for helpers
	
	echo "Content-type: text/HTML"	# tell browser type of output
	echo ""				# end of header	
	
	# HTML header to preserve report back formating
	printf "<!doctype html>\n<html>\n<body>\n<pre><code>"
	
	retrieve_clean_and_check_input

	check_data_dir
	
	delete_item

	# footer
	printf "</code></pre>\n</body>\n</html>\n"
