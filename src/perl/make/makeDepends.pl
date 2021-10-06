#!/usr/bin/perl 

#
# makeDepends.pl
#
# Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
# intellectual property of Joshua S. English, all rights reserved.
#
# A simple Perl script to profile the C and C++ source trees to determine what
# objects, binaries and unit tests to build via make.
#
# Written by Josh English.
#

# define script configuration

my $makeDir = "conf/make";

my $baseDirC = "src/c";
my $baseDirCpp = "src/cpp";

my $targetTypeC = "*.c";
my $targetTypeCpp = "*.cpp";

# execute depedency creation

open(fdTest, ">", "$makeDir/makefile.tests");
open(fdTestDep, ">", "$makeDir/makefile.test.dependency");

print(fdTest "TESTS\t\t\t\t\t\t= \\\n");

open(fdApp, ">", "$makeDir/makefile.apps");
open(fdAppDep, ">", "$makeDir/makefile.app.dependency");

print(fdApp "APPS\t\t\t\t\t\t= \\\n");

if(-d $baseDirC) {
	scanSourcePackages($baseDirC, $targetTypeC);
}
if(-d $baseDirCpp) {
	scanSourcePackages($baseDirCpp, $targetTypeCpp);
}

close(fdTest);
close(fdTestDep);
close(fdApp);
close(fdAppDep);

exit 0;

# define subroutines

sub scanSourcePackages()
{
	my $baseDir = shift(@_);
	my $targetType = shift(@_);
	my $scanList = "";
	my @packageList = ();

	$scanList = `ls -1 $baseDir`;

	push(@packageList, split(/\n/, $scanList));

	foreach my $package (@packageList) {
		my $packageDir = $baseDir . "/" . $package;

		if($package eq "CVS") {
			next;
		}

		print "Package: $package ($packageDir)\n";

		if(! -d $packageDir) {
			print("error - unable to locate directory '$packageDir'.\n");
			exit 1;
		}

		scanSourceFiles($package, $packageDir, $targetType);
	}
}

sub scanSourceFiles()
{
	my $packageName = shift(@_);
	my $baseDir = shift(@_);
	my $targetType = shift(@_);
	my $scanList = "";
	my @fileList = ();

	$scanList = `find $baseDir -name "$targetType" | sort -n`;

	push(@fileList, split(/\n/, $scanList));

	open(fdObj, ">", "$makeDir/makefile.$packageName.objects");
	open(fdObjDep, ">", "$makeDir/makefile.$packageName.object.dependency");

	my $temp = uc($packageName);
	print(fdObj "$temp" . "_OBJS\t\t\t\t\t\t= \\\n");

	foreach my $file (@fileList) {
		if(($file =~ /\/\./) || ($file =~ /^\./)) {
			next;
		}

		print "\t ...source: $file\n";
	
		my ($type,
			$directory,
			$basename,
			$directory2,
			$directory3,
			$extension) = fileInfo($file);

#		printf("\t type       : $type\n");
#		printf("\t directory  : $directory\n");
#		printf("\t basename   : $basename\n");
#		printf("\t directory2 : $directory2\n");
#		printf("\t directory3 : $directory3\n");
#		printf("\t extension  : $extension\n");

		if($type eq "object") {
			print(fdObj
				"\t\t\$(SRC_PATH)/$extension/$directory3/$basename.o \\\n");
			print(fdObjDep
				getObjDeps($file, $basename, $directory, $extension));
		}
		elsif($type eq "unit test") {
			print(fdTest
				"\t\t\$(TEST_PATH)/$basename\$(APP_EXT) \\\n");
			print(fdTestDep
				getTestDeps($file, $basename, $extension));
		}
		elsif($type eq "application") {
			print(fdApp "\t\t\$(APP_PATH)/$basename\$(APP_EXT) \\\n");
			print(fdAppDep
				getAppDeps($file, $basename, $directory, $extension));
		}
	}

	close(fdObj);
	close(fdObjDep);
}

sub fileInfo
{
	my $file = shift(@_);
	my $type = "";

	my ($directory,
		$basename,
		$directory2,
		$directory3,
		$extension) = fileParts($file);

	if($basename =~ /unitTest/) {
		$type = "unit test";
	}
	elsif(-f "$directory/$basename.h") {
		$type = "object";
	}
	else {
		$type = "application";
	}

	return ($type, $directory, $basename, $directory2, $directory3, $extension);
}

sub fileParts
{
	my $filename = shift(@_);

	my $lastDot = rindex($filename, ".");
	my $lastSlash = rindex($filename, "/");

	my $secondLastSlash = rindex($filename, "/", $lastSlash - 1);
	if ((-1 == $lastDot) || (-1 == $lastSlash)) {
		return "";
	}

	my $directory = substr($filename, 0, $lastSlash);
	my $directory2 = substr($filename, $secondLastSlash + 1,
		($lastSlash - $secondLastSlash) - 1);

	my @dirs = split(/\//, $directory);
	my $directory3 = join("/", @dirs[2 .. (scalar(@dirs) - 1)]);

	my $basename = substr($filename, $lastSlash + 1,
		($lastDot - ($lastSlash + 1)));

	my $extension = substr($filename, $lastDot + 1);

	return ($directory, $basename, $directory2, $directory3, $extension);
}

sub getObjDeps()
{
	my $sourceFilename = shift(@_);
	my $basename = shift(@_);
	my $directory = shift(@_);
	my $extension = shift(@_);

	my @dependencies = ();

	@dependencies = getProjectHeaders($sourceFilename, @dependencies);

	my $deps = "$directory/$basename.o: \\\n";

	foreach my $entry (@dependencies) {
		my $sourceInclude = "";
		if(-f "$baseDirC/$entry") {
			$sourceInclude = "$baseDirC/$entry";
		}
		elsif(-f "$baseDirCpp/$entry") {
			$sourceInclude = "$baseDirCpp/$entry";
		}

		if($sourceInclude) {
			$deps .= "\t$sourceInclude \\\n";
			my $sourceFile = correspondingSourceFile($entry);
			if($sourceFile) {
				$deps .= "\t$sourceFile \\\n";
			}
		}
	}

	chop($deps);
	chop($deps);
	chop($deps);
	$deps .= "\n";

	return $deps;
}

sub getTestDeps()
{
	my $sourceFilename = shift(@_);
	my $basename = shift(@_);
	my $extension = shift(@_);
	my $sourceFilenameNew = substr($sourceFilename, 4);
	my $SDLMain = 0;

	my $compiler = "\$(CC)";
	if ($extension eq "cpp") {
		$compiler = "\$(CC++)";
	}

	my @dependencies = ();

	@dependencies = getProjectHeaders($sourceFilename, @dependencies);

	my $deps = "\$(TEST_PATH)/$basename\$(APP_EXT): \\\n";

	foreach my $entry (@dependencies) {
		my $sourceInclude = "";
		if(-f "$baseDirC/$entry") {
			$sourceInclude = "$baseDirC/$entry";
		}
		elsif(-f "$baseDirCpp/$entry") {
			$sourceInclude = "$baseDirCpp/$entry";
		}

		if($sourceInclude) {
			$deps .= "\t$sourceInclude \\\n";
			my $sourceFile = correspondingSourceFile($entry);
			if($sourceFile) {
				$deps .= "\t$sourceFile \\\n";
			}
		}
	}

	$deps .= "\t\t\$(SRC_PATH)/$sourceFilenameNew \\\n";
	$deps .= "\t\t\$(LIBS)\n";
	$deps .= "\t$compiler \$(COMPILE_FLAGS) " .
			"\$(SRC_PATH)/$sourceFilenameNew \\\n";
	$deps .= "\t\t-o \$(TEST_PATH)/$basename\$(APP_EXT) \$(LIBRARY_FLAGS) \n";

	return $deps;
}

sub getAppDeps()
{
	my $sourceFilename = shift(@_);
	my $basename = shift(@_);
	my $directory = shift(@_);
	my $extension = shift(@_);
	my $sourceFilenameNew = substr($sourceFilename, 4);
	my $SDLMain = 0;

	my $compiler = "\$(CC)";
	if ($extension eq "cpp") {
		$compiler = "\$(CC++)";
	}

	my @dependencies = ();

	@dependencies = getProjectHeaders($sourceFilename, @dependencies);

	my $deps = "\$(APP_PATH)/$basename\$(APP_EXT): \\\n";

	foreach my $entry (@dependencies) {
		my $sourceInclude = "";
		if(-f "$baseDirC/$entry") {
			$sourceInclude = "$baseDirC/$entry";
		}
		elsif(-f "$baseDirCpp/$entry") {
			$sourceInclude = "$baseDirCpp/$entry";
		}

		if($sourceInclude) {
			$deps .= "\t$sourceInclude \\\n";
			my $sourceFile = correspondingSourceFile($entry);
			if($sourceFile) {
				$deps .= "\t$sourceFile \\\n";
			}
		}
	}

	$deps .= "\t\t\$(SRC_PATH)/$sourceFilenameNew \\\n";
	$deps .= "\t\t\$(LIBS)\n";
	$deps .= "\t$compiler \$(COMPILE_FLAGS) " .
			"\$(SRC_PATH)/$sourceFilenameNew \\\n";
	$deps .= "\t\t-o \$(APP_PATH)/$basename\$(APP_EXT) \$(LIBRARY_FLAGS) \n";

	return $deps;
}

sub getProjectHeaders()
{
	my $sourceFilename = shift(@_);
	my @dependencies = @_;

	open(my $fileHandle, "<", $sourceFilename);

	while(<$fileHandle>) {
		if(/^#include\s+\"(.*)\"/) {
			my $include = $1;
			my $sourceInclude = "";
			if(-f "$baseDirC/$include") {
				$sourceInclude = "$baseDirC/$include";
			}
			elsif(-f "$baseDirCpp/$include") {
				$sourceInclude = "$baseDirCpp/$include";
			}

			if($sourceInclude ne "") {
				my @temp = grep {$_ eq $include} @dependencies;
				if($#temp < 0) {
					push(@dependencies, $include);
					@dependencies = getProjectHeaders($sourceInclude,
						@dependencies);
				}
			}
		}
	}

	return @dependencies;
}

sub correspondingSourceFile
{
	my $hFile = shift(@_);

	my $cFile = substr($hFile, 0, length($hFile) - 1) . "c";
	my $cppFile = substr($hFile, 0, length($hFile) - 1) . "cpp";

	my $sourceFile = "";

	if (-f "$baseDirC/$cFile") {
		$sourceFile = "$baseDirC/$cFile";
	}
	elsif (-f "$baseDirCpp/$cppFile") {
		$sourceFile = "$baseDirCpp/$cppFile";
	}
	else {
		$sourceFile = "";
	}

	return ($sourceFile);
}

sub dependencyRootDir
{
	my $dependency = shift;

	if (-f "$baseDirC/$dependency") {
		return "c";
	}
	elsif (-f "$baseDirCpp/$dependency") {
		return "cpp";
	}
	else {
		return "";
	}
}

