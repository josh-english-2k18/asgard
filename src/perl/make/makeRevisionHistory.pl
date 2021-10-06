#!/usr/bin/perl

#
# makeRevisionHistory.pl
#
# Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
# intellectual property of Joshua S. English, all rights reserved.
#
# A simple Perl script to convert CVS log history into a revision-
# history document.
#
# Written by Josh English.
#

# define usages

use Tie::File;
use IO::Pipe;

# execute revision-history parsing

executeRevisionHistory();

# define subroutines

sub executeRevisionHistory()
{
	my $append;

	if(@ARGV < 1 || $ARGV[0] < 1){
		die "usage: makeRevisionHsitory.pl [number of days to log]\n";
	}

	if($ARGV[0] > 1){
		$append = "s";
	}
	else{
		$append = "";
	}

	my $exec_line = "cvs log -d \">" . $ARGV[0] . " day" . "$append" . " ago\" . " . " > .revision.history.tempfile 2>/dev/null";

	my @execCommand = ($exec_line);
	system(@execCommand);

	my $notesfd;
	my $stepper;
	my $ii = 0;
	my @info;
	my @date;
	my @author;
	my $person;
	my $time;
	my %entries;
	my %counts;
	my $reading = 0;
	my %known;

	tie(@NOTES, 'Tie::File', "./doc/revision.history");
	open($notesfd, '>>', "./doc/revision.history") or die "could not open revision.history for read/write"; 

	my $max_ver;
	my $mid_ver;
	my $min_ver;
	my $dayz;
	my @dayparts;
	my $ii = @NOTES;

	while(! ($NOTES[$ii-1] =~ /\d\.\d\.\d+/)){ 
		$ii--;
	}
	($version) = ($NOTES[$ii-1] =~ /(\d\.\d\.\d+)/);

#print "version: $version (from $NOTES[$ii-1])\n";
	
	($max_ver, $mid_ver, $min_ver) = split(/\./, $version);

	# force the version-strings to be numbers

	$max_ver++;
	$max_ver--;
	$mid_ver++;
	$mid_ver--;
	$min_ver++;
	$min_ver--;

#print "max: $max_ver, mid: $mid_ver, min $min_ver\n";	

	tie (@fileLine, 'Tie::File', ".revision.history.tempfile");

	$ii = 0;

	while($ii < @fileLine){
		# skip delmiter line(s)

		while(($fileLine[$ii] !~ /----------------------------/) &&
				($ii < @fileLine)) {
			$ii++;
		}

		# skip separator line

		$ii++;

		# skip revision line

		$ii++;

		if($fileLine[$ii] eq "") {
			last;
		}

		# get date and author

		@info = split(/;/, $fileLine[$ii]);
		#date is info[0] author is info[1]
		@date = split(/:/, $info[0]);
		$dayz = substr($date[1], 1, 10);
		@dayparts = split(/\//, $dayz);
		$day = $dayparts[1] . "/" . $dayparts[2] . "/20" . substr($dayparts[0], 2, 2);		

		$time = $date[1] . ":" . $date[2] . ":" . $date[3];
		@author = split(/:/, $info[1]);
		$person = $author[1];

		if(!defined($entries{$time})) {
#print "line[" . $ii . "] is '" . $fileLine[$ii] . "'\n";
#print "\t time   : " . $time . "\n";
#print "\t person : " . $person . "\n";
			my $buffer = "[$day]  \$    $person \$";
			while(length($buffer) lt 28) {
				$buffer = $buffer . " ";
			}
			$entries{$time} = $buffer;#"[$day]  \$    $person";
			$counts{$time} = 0;
#print "\t\t ADDED ENTRY '" . $entries{$time} . "'\n";
		}

		$ii++;

		while(($fileLine[$ii] !~ /----------------------------/) &&
				($fileLine[$ii] !~ /=============================================================================/) &&
				($ii < @fileLine)) {
			if($known{$fileLine[$ii]} != 1) {
#print "\t\t line[" . $ii . "] is '" . $fileLine[$ii] . "'\n";
#				$entries{$time} = $entries{$time} . "   " . $fileLine[$ii] . "\n[$day]  \$    $person";
				$entries{$time} = $entries{$time} . " " . $fileLine[$ii];
#print "\t\t ADDED LINE '" . $entries{$time} . "'\n";
				$known{$fileLine[$ii]} = 1;
				$ii++;
				$counts{$time} += $counts{$time} + 1;
			}
			else {
				$ii++;
			}
		}

		if($counts{$time} eq 0) {
#print "\t\t REMOVED ENTRY '" . $entries{$time} . "'\n";
			delete($entries{$time});
		}
	}

	my @parts;	
	seek($notesfd, 0, SEEK_END);
	$min_ver++;
	foreach $key (sort (keys(%entries))) {
		if($entries{$key} !~ /\[\d\d[\w\W]*/) {
			next;
		}
		else {
			@parts = split(/\$/, $entries{$key});

			$datestamp = $parts[0];
			$buffer = $parts[1] . '$' . $parts[2];
#print "entry =>$entries{$key}\n";
#print "datestamp =>$datestamp\n";
#print "buffer =>$buffer\n";

			@parts = split(/\$/, $buffer);

			$username = $parts[0];
#print "username =>$username\n";
			@separate = split(/\./, $parts[1]);
			for my $thing (@separate) {
#print "$datestamp, $username, ---->$thing<----\n";
				@parts = split(/\$/, $thing);
				for my $lines (@parts) {
					$lines =~ s/^\s+//;
					$lines = $lines . ".";
#print "---->$lines<----\n";
					if($min_ver < 10) {
						$output = "$datestamp $max_ver.$mid_ver.0$min_ver $username ";
					}
					else {
						$output = "$datestamp $max_ver.$mid_ver.$min_ver $username ";
					}

					while(length($output) lt 37) {
						$output = $output . " ";
					}

					print $notesfd "$output $lines\n\n";

					$min_ver++;
					if($min_ver eq 100) {
						$mid_ver++;
						$min_ver = 0;
					}
					if($mid_ver eq 10) {
						$max_ver++;
						$mid_ver = 0;
					}
				}
			}
		}
	}
	close($notesfd);

	unlink(".revision.history.tempfile");
}

