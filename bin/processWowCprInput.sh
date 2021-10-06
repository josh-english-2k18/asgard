#!/bin/bash

#
# processWowCprInput.sh
#
# Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
# intellectual property of Joshua S. English, all rights reserved.
#
# A bash shell script to process combat logs with the 'wowcpr' application from
# the processing location to the done location.
#
# Written by Josh English.
#

#
# script settings
#

baseDir="/home/josh/files/projects/asgard"

abortDir="assets/data/wowcprGarbage"

inputDir="assets/data/wowcprInput"

outputDir="assets/data/wowcprOutput"


#
# script functions
#

logf()
{
	if [ "$1" != "" ]
	then
		logLevel="$1"
	else
		logLevel="UNKNOWN"
	fi
	if [ "$2" != "" ]
	then
		logf="$2"
	else
		logf="(unknown)"
	fi

	timestamp=`date +%Y.%m.%d:%H:%M:%S`
	echo "[$timestamp] {$logLevel} $logf"

	if [ "$logLevel" = "PANIC" ]
	then
		exit 1
	fi
}

moveFile()
{
	result="passed"

	if [ "$1" = "" ]
	then
		logf "PANIC" "unknown file '$1'"
	fi

	sourceFilename="$1"

	if [ ! -f "$sourceFilename" ]
	then
		logf "PANIC" "unable to locate file '$sourceFilename'"
	fi

	destFilename="$2"

	if [ -f "$destFilename" ]
	then
		logf "PANIC" "file '$destFilename' already exists"
	fi

	mv "$sourceFilename" "$destFilename" 2>/dev/null

	if [ "$?" != "0" ]
	then
		logf "ERROR" "failed to move file '$sourceFilename'"
		result="failed"
	elif [ ! -f "$destFilename" ]
	then
		logf "ERROR" "failed to move file to '$destFilename'"
		result="failed"
	fi
}


#
# perform the file processing
#

# check that the directories exist

if [ ! -d "$baseDir" ]
then
	logf "PANIC" "directory '$baseDir' does not exist"
fi

if [ ! -d "$abortDir" ]
then
	logf "PANIC" "directory '$abortDir' does not exist"
fi

if [ ! -d "$inputDir" ]
then
	logf "PANIC" "directory '$inputDir' does not exist"
fi

if [ ! -d "$outputDir" ]
then
	logf "PANIC" "directory '$outputDir' does not exist"
fi

if [ ! -f "$baseDir/bin/wowcpr" ]
then
	logf "PANIC" "unable to locate '$baseDir/bin/wowcpr' application"
fi

# process the input argument(s)

processCounter=0
fileProcessLimit=0

if [ "$1" != "" ]
then
	fileProcessLimit="$1"
fi

if [ $fileProcessLimit -gt 0 ]
then
	logf "INFO" "limiting file-processing to $fileProcessLimit file(s)"
fi

# obtain the list of files to process

logf "INFO" "processing files from '$inputDir'"

cd "$baseDir"

fileList=`ls -1 "$inputDir" | grep "\.log\$"`

# perform processing

for file in $fileList
do
	logf "INFO" "processing file '$file'"

	./bin/wowcpr "$inputDir/$file" > "$inputDir/$file.report" \
		2>.processfile.temp

	if [ "$?" != "0" ]
	then
		logf "ERROR" \
			"failed to process file '$file' with `head -n 1 .processfile.temp`"
		logf "INFO" "moving file to abort location '$abortDir/$file'"
		moveFile "$inputDir/$file" "$abortDir/$file"
		moveFile "$inputDir/$file.meta" "$abortDir/$file.meta"
		moveFile "$inputDir/$file.report" "$abortDir/$file.report"
	else
		logf "INFO" "moving file to done location '$outputDir/$file'"
		moveFile "$inputDir/$file" "$outputDir/$file"
		moveFile "$inputDir/$file.meta" "$outputDir/$file.meta"
		moveFile "$inputDir/$file.report" "$outputDir/$file.report"
	fi

	rm -f .processfile.temp

	processCounter=`expr $processCounter + 1`
	if [ $fileProcessLimit -gt 0 -a $processCounter -ge $fileProcessLimit ]
	then
		break
	fi
done

logf "INFO" "$processCounter files from '$inputDir' have been processed"


#
# end of script
#

exit 0

