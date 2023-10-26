#!/usr/bin/env perl

use strict;
use warnings;
use Getopt::Long;

my $usage = <<END;
Usage: $0 [--mode=instr|bb|opcode] [-n output_sz] [-c window_sz] < trace.txt
Options:
    --mode=instr        -- find most frequent instructions (default)
    --mode=bb           -- find most frequent basic blocks
    --mode=opcode       -- find most frequent opcode sequences

    -n output_sz=0      -- number of entries in the output (0 = print all entries)
    -c window_sz=1      -- number of opcodes in a sequence for --mode=opcode
END

sub usage_error { die $usage; }

my $mode = "instr";
my $output_sz = 0; # no limit
my $window_sz = 1;

GetOptions(
"mode=s" => \$mode,
"n=i" => \$output_sz,
"c=i" => \$window_sz
) or usage_error;

if (!($mode eq "instr" or $mode eq "bb" or $mode eq "opcode")) {
  print STDERR "Error: invalid mode\n";
  usage_error;
}

sub get_next_opcode {
  while (my $line = <STDIN>) {
    next if $line =~ /^;/;
    return $1 if $line =~ /^%\S+ = (\S+) /;
    return $1 if $line =~ /^(\S+) /;
    die "invalid trace entry '$line'";
  }
}

if ($mode eq "opcode") {
  # gather opcodes
  my @opcodes;
  while (my $opcode = get_next_opcode) {
    push @opcodes, $opcode
  }

  # calculate occurencies
  my %count;
  my $sequence_count = $#opcodes - $window_sz;
  for (my $i = 0; $i < $sequence_count; ++$i) {
    my @sequence = @opcodes[$i .. ($i + $window_sz - 1)];
    ++$count{join " + ", @sequence};
  }

  # transform into array
  my @sequences;
  for my $sequence (keys %count) {
    push @sequences, { opcodes => $sequence, count => $count{$sequence} }
  }

  # sort by frequency
  @sequences = sort { $b->{count} <=> $a->{count}; } @sequences;

  # leave only $output_sz patterns
  if ($output_sz > 0 && $#sequences > $output_sz) {
    @sequences = @sequences[0 .. ($output_sz - 1)];
  }

  # pretty printing
  my $fmt = "%-10s %s\n";
  printf $fmt, "FREQUENCY", "PATTERN";
  for my $sequence (@sequences) {
    my $percent = sprintf "%.2f%%", $sequence->{count} / $sequence_count * 100;
    printf $fmt, $percent, $sequence->{opcodes};
  }
} else { # instr and bb modes
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
}
