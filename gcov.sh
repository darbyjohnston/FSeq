#!/bin/sh

gcnoList=`find $1 -name "*.gcno" -and \( -not -name "CMakeC*CompilerId.gcno" \)`
echo "gcnoList="$gcnoList
for gcno in $gcnoList
do
    srcName=`basename -s .gcno $gcno`
    echo "srcName="$srcName
    src=`find $1 -name $srcName`
    echo "src="$src
    gcov -o $gcno $src
done
