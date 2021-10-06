#!/bin/bash

path="."

if [ "$1" != "" ]
then
	path="$1"
fi

cvs update -d -P "$path" 2>&1 | grep -i -v "cvs update"
if [ "$?" != "0" ]
then
	echo "error - cvs update failed."
	exit 1
fi

exit 0

