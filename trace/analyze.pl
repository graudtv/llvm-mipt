#!/usr/bin/env perl

use strict;
use warnings;
use Getopt::Long;

sub usage_error { die "Usage: $0 [--mode=instr|bb] [-n output_sz] < trace.txt\n" }

my $mode = "instr";
my $output_sz = 0; # no limit

GetOptions(
"mode=s" => \$mode,
"n=i" => \$output_sz
) or usage_error;

if (!($mode eq "instr" or $mode eq "bb")) {
  print STDERR "Error: invalid mode\n";
  usage_error;
}

# calculate occurencies
my %count;
my $line_count = 0;
while (my $line = <STDIN>) {
  if (($mode eq "bb" && $line =~ /^; function '\S+' BB '\S+'/) ||
       $mode eq "instr" && $line !~ /^;/) {
    chomp $line;
    ++$count{$line};
    ++$line_count;
  }
}

# transform into array
my @patterns;
for my $line (keys %count) {
  push @patterns, { line=>$line, count=>$count{$line} }
}

# sort by frequency
@patterns = sort { $b->{count} <=> $a->{count}; } @patterns;

# leave only $output_sz patterns
if ($output_sz > 0 && $#patterns > $output_sz) {
  @patterns = @patterns[0 .. ($output_sz - 1)];
}

# pretty printing
my $fmt = "%-10s %s\n";
printf $fmt, "FREQUENCY", "PATTERN";
for my $pattern (@patterns) {
  my $percent = sprintf "%.2f%%", $pattern->{count} / $line_count * 100;
  printf $fmt, $percent, $pattern->{line};
}

