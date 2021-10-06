#!/bin/bash

#
# executeUnitTests.sh
#
# Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
# intellectual property of Joshua S. English, all rights reserved.
#
# A simple bash shell script to execute the Asgard unit tests.
#
# Written by Josh English.
#

fileExtension=""

if [ "`uname -a | grep -i "cygwin"`" != "" ]
then
	fileExtension=".exe"
fi

fileList=`ls -1 unit/*$fileExtension | cut -d'/' -f2 | sort -n | uniq`

for file in $fileList
do
	if [ -d unit/$file ]
	then
		continue
	fi

	if [ ! -f unit/$file ]
	then
		echo "error - unable to locate 'unit/$file'."
		continue
	fi

	echo -n "Executing 'unit/$file'..."

	./unit/$file > .unitOutput 2>&1

	if [ $? != 0 ]
	then
		mv .unitOutput $file.output
		echo "FAILED, see $file.output for details."
	else
		rm -f .unitOutput
		echo "PASSED."
	fi
done

exit 0

