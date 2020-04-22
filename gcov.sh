#!/bin/sh

gcnoList=`find $1 -name "*.gcno" -and \( -not -name "CMakeC*CompilerId.gcno" \)`
for gcno in $gcnoList
do
	srcName=`basename -s .gcno $gcno`
	src=`find $1 -name $srcName`
	gcov -o $gcno $src
done
