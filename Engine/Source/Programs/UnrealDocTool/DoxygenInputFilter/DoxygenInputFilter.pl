#!/usr/bin/perl -w
#use strict;

# Find the first "//" or "/*" on the line.
# If it's immediately followed by "~" (our "don't comment" code), ignore it.
# If it's a "///" or "/**" (Doxygen's "comment" code), ignore it.
# Otherwise, transform it into the appropriate Doxygen "comment" code.
# For .cpp files, ignore all comment-modification code as it's causing undesired comments (e.g. copyright notices) to be captured.

my $line = "";
my $lastclassname;
my $commentposition;
my $tempvalue;
open(FILE1, $ARGV[0]) || die "  error: $!\n";
while (<FILE1>)
{
    $line = $_;
	
	if ($line =~ /\/\/ Copyright .+ Epic Games, Inc. All Rights Reserved./)
	{
		# Discard copyright line.
		$line = " ";
	}
	else
	{
		if ($line =~ /class .+ :.*/)
		{
			$lastclassname = substr($line, index($line, "class ") + 6, index($line, ":") - 6);
		}
		else
		{
			if (index($line, "GENERATED_UCLASS_BODY()") >= 0)
			{
				$line = "GENERATED_BODY() public: " . $lastclassname . "(const FObjectInitializer& ObjectInitializer);\n";
			}
		}
		$commentposition = index($line, "\/\/");
		$tempvalue = index($line, "\/*");
		if ($commentposition < 0)
		{
			$commentposition = $tempvalue;
		}
		elsif ($tempvalue >= 0)
		{
			$commentposition = $commentposition < $tempvalue ? $commentposition : $tempvalue;
		}
		
		if ($commentposition >= 0)
		{
			if ((length $line) > ($commentposition + 2))
			{
				$tempvalue = substr($line, $commentposition + 2, 1);
				if (!($tempvalue eq "~") && !($tempvalue eq substr($line, $commentposition + 1, 1)))
				{
					substr($line, $commentposition + 1, 0) = substr($line, $commentposition + 1, 1);
				}
			}
			# This elsif indicates an error - the line didn't end with "\n".
			# If we find this happening, we should start appending "\n" on our own.
			#elsif ((length $line) > ($commentposition + 1))
			#{
			#    substr($line, $commentposition + 1, 0) = substr($line, $commentposition + 1, 1);
			#}
		}
	}
	print $line;
}
