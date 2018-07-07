#!/bin/sh

	printf "Content-Type: text/plain\r\n\r\n"

	echo "HEre are env vars"
	env
	echo ""
	echo ""
	echo here is what comes in via stdin:
	cat
