#!/bin/bash

#
# stageWowCprInput.sh
#
# Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
# intellectual property of Joshua S. English, all rights reserved.
#
# A bash shell script to stage combat logs from an upload location to the
# processing location.
#
# Written by Josh English.
#

#
# script settings
#

baseDir="/home/josh/files/projects/asgard"

stageDir="assets/data/wowcprFtp"

archiveDir="assets/data/wowcprArchive"

abortDir="assets/data/wowcprGarbage"

processDir="assets/data/wowcprInput"


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

copyFile()
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
		logf "WARNING" "file '$destFilename' already exists"
	fi

	cp "$sourceFilename" "$destFilename" 2>/dev/null

	if [ "$?" != "0" ]
	then
		logf "ERROR" "failed to copy file '$sourceFilename'"
		result="failed"
	elif [ ! -f "$destFilename" ]
	then
		logf "ERROR" "failed to copy file to '$destFilename'"
		result="failed"
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

getFileType()
{
	result=""

	if [ "$1" = "" ]
	then
		logf "PANIC" "unknown file '$1'"
	fi

	filename="$1"

	if [ ! -f "$filename" ]
	then
		logf "PANIC" "unable to locate file '$filename'"
	fi

	fileType=`file "$filename"`

	if [ "`echo "$fileType" | grep "Zip archive"`" != "" ]
	then
		result="zip"
	elif [ "`echo "$fileType" | grep "gzip compressed"`" != "" ]
	then
		result="gzip"
	elif [ "`echo "$fileType" | grep "tar archive"`" != "" ]
	then
		result="tar"
	elif [ "`echo "$fileType" | grep -i "rar archive"`" != "" ]
	then
		result="rar"
	elif [ "`echo "$fileType" | grep -i "english text"`" != "" ]
	then
		result="text"
	else
		result="unknown"
	fi
}

unzipFile()
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
		logf "PANIC" "zip file '$destFilename' already exists"
	fi

	unzip -p "$sourceFilename" > "$destFilename" 2>/dev/null

	if [ "$?" != "0" ]
	then
		logf "ERROR" "failed to unzip file '$sourceFilename'"
		result="failed"
	elif [ ! -f "$destFilename" ]
	then
		logf "ERROR" "failed to create unzipped file '$destFilename'"
		result="failed"
	fi
}

untarFile()
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
		logf "PANIC" "zip file '$destFilename' already exists"
	fi

	tar -x -O -f "$sourceFilename" > "$destFilename" 2>/dev/null

	if [ "$?" != "0" ]
	then
		logf "ERROR" "failed to untar file '$sourceFilename'"
		result="failed"
	elif [ ! -f "$destFilename" ]
	then
		logf "ERROR" "failed to create untarred file '$destFilename'"
		result="failed"
	fi
}

degzipFile()
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
		logf "PANIC" "zip file '$destFilename' already exists"
	fi

	gzip -c -d "$sourceFilename" > .stagefile.temp 2>/dev/null

	if [ "$?" != "0" ]
	then
		logf "ERROR" "failed to gzip decompress file '$sourceFilename'"
		result="failed"
		rm -f .stagefile.temp
		return
	fi

	getFileType ".stagefile.temp"
	fileType="$result"

	if [ "$fileType" = "text" ]
	then
		logf "INFO" "moving text file to '$destFilename'"
		moveFile ".stagefile.temp" "$destFilename"
	elif [ "$fileType" = "tar" ]
	then
		logf "INFO" "untarring tar file to '$destFilename'"
		untarFile ".stagefile.temp" "$destFilename"
	else
		logf "ERROR" \
			"unhandled type '$fileType' for contents of gzip file '$sourceFilename'"
		result="failed"
	fi

	rm -f .stagefile.temp

	if [ ! -f "$destFilename" ]
	then
		logf "ERROR" "failed to create gzip decompress file '$destFilename'"
		result="failed"
	fi
}

unrarFile()
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
		logf "PANIC" "rar file '$destFilename' already exists"
	fi

	unrar p -inul "$sourceFilename" | strings > "$destFilename" 2>/dev/null

	if [ "$?" != "0" ]
	then
		logf "ERROR" "failed to unrar file '$sourceFilename'"
		result="failed"
	elif [ ! -f "$destFilename" ]
	then
		logf "ERROR" "failed to create unrared file '$destFilename'"
		result="failed"
	fi
}

createMetafile()
{
	result=""

	if [ "$1" = "" ]
	then
		logf "PANIC" "unknown file '$1'"
	fi

	filename="$1"

	if [ -f "$filename" ]
	then
		logf "WARNING" "meta-data file '$filename' already exists"
	fi

	parentFilename="$2"

	> $filename

	echo "#" >> $filename
	echo "# $parentFilename.meta" >> $filename
	echo "#" >> $filename
	echo "# File auto-generated on `date +%Y/%m/%d` at `date +%H:%M:%S`." >> $filename
	echo "#" >> $filename
	echo "" >> $filename
	echo "#" >> $filename
	echo "# general properties" >> $filename
	echo "#" >> $filename
	echo "" >> $filename
	echo "[general]" >> $filename
	echo "ownerEmailAddress=andaas@hossguild.net" >> $filename
	echo "regionCode=US" >> $filename
	echo "realmName=Draenor" >> $filename
	echo "fileYear=2009" >> $filename
	echo "fileType=Raid" >> $filename
	echo "filename=$parentFilename" >> $filename
	echo "fileDescription=World of Warcraft Combat Log" >> $filename
	echo "" >> $filename
}


#
# perform the file staging
#

# check that the directories exist

if [ ! -d "$baseDir" ]
then
	logf "PANIC" "directory '$baseDir' does not exist"
fi

if [ ! -d "$stageDir" ]
then
	logf "PANIC" "directory '$stageDir' does not exist"
fi

if [ ! -d "$archiveDir" ]
then
	logf "PANIC" "directory '$archiveDir' does not exist"
fi

if [ ! -d "$abortDir" ]
then
	logf "PANIC" "directory '$abortDir' does not exist"
fi

if [ ! -d "$processDir" ]
then
	logf "PANIC" "directory '$processDir' does not exist"
fi

# process the input argument(s)

stageCounter=0
fileStageLimit=0

if [ "$1" != "" ]
then
	fileStageLimit="$1"
fi

if [ $fileStageLimit -gt 0 ]
then
	logf "INFO" "limiting file-staging to $fileStageLimit file(s)"
fi

# obtain the list of files to stage

logf "INFO" "staging files from '$stageDir'"

cd "$baseDir"

fileList=`ls -1 "$stageDir"`

# perform staging

for file in $fileList
do
	# make sure the file is not a directory

	if [ -d "$stageDir/$file" ]
	then
		continue
	fi

	# obtain the file type

	getFileType "$stageDir/$file"
	fileType="$result"

	logf "INFO" "file '$file' is type '$fileType'"

	# create the destination file name

	counter=0
	timestamp=`date +%Y.%m.%d.%H.%M.%S`
	newFilename="$file.$timestamp.n$counter.log"

	while [ -f "$processDir/$newFilename" ]
	do
		counter=`expr $counter + 1`
		newFilename="$file.$timestamp.n$counter.log"
	done

	# extract the file

	if [ "$fileType" = "zip" ]
	then
		logf "INFO" "extracting zip '$file' to '$newFilename'"
		unzipFile "$stageDir/$file" "$processDir/$newFilename"
	elif [ "$fileType" = "tar" ]
	then
		logf "INFO" "extracting tar '$file' to '$newFilename'"
		untarFile "$stageDir/$file" "$processDir/$newFilename"
	elif [ "$fileType" = "gzip" ]
	then
		logf "INFO" "extracting gzip '$file' to '$newFilename'"
		degzipFile "$stageDir/$file" "$processDir/$newFilename"
	elif [ "$fileType" = "rar" ]
	then
		logf "INFO" "extracting rar '$file' to '$newFilename'"
		unrarFile "$stageDir/$file" "$processDir/$newFilename"
	elif [ "$fileType" = "text" ]
	then
		logf "INFO" "copying text '$file' to '$newFilename'"
		copyFile "$stageDir/$file" "$processDir/$newFilename"
	else
		result="failed"
	fi

	# move the file to the appropriate location

	if [ "$result" = "failed" ]
	then
		logf "INFO" "moving file to abort location '$abortDir/$file'"
		moveFile "$stageDir/$file" "$abortDir/$file"
	else
		# obtain or create a meta-data file

		if [ -f "$stageDir/$file.meta" ]
		then
			logf "INFO" "copying meta-data file '$stageDir/$file.meta'"
			copyFile "$stageDir/$file.meta" "$processDir/$newFilename.meta"
			logf "INFO" \
				"moving meta-data file '$stageDir/$file.meta' to archive"
			moveFile "$stageDir/$file.meta" "$archiveDir/$file.meta"
		else
			logf "WARNING" \
				"creating default meta-data file '$processDir/$newFilename.meta'"
			createMetafile "$processDir/$newFilename.meta" "$newFilename"
		fi

		logf "INFO" "moving file to archive location '$archiveDir/$file'"
		moveFile "$stageDir/$file" "$archiveDir/$file"
	fi

	stageCounter=`expr $stageCounter + 1`
	if [ $fileStageLimit -gt 0 -a $stageCounter -ge $fileStageLimit ]
	then
		break
	fi
done

logf "INFO" "$stageCounter files from '$stageDir' have been staged"


#
# end of script
#

exit 0

