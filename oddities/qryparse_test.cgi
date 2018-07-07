#!/bin/sh
#	a demo for qryparse
#

echo "Content-type: text/plain"
echo ""
exec 2>&1
set -x


echo "the query string is $QUERY_STRING"

echo "this is the output of qryparse"
./qryparse

echo ""

echo "this is cleaned qryparse"
./qryparse | ./clean_qryparse

#echo "this is the output of clean_qryparse"
#./check_qryparse $(./qryparse)

