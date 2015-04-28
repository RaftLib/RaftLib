#!/usr/bin/env perl
use strict;
use warnings;

sub getline( $ );
sub getlast( $ );

my @list;
for( my $i = 1; $i < 20; $i++ )
{
   my $repeat = 2;
   while( $repeat-- )
   {
      print STDERR "Executing $i processes, repeat # $repeat\n";
      my $ret_val = `/usr/bin/time -p ./osPartitionTest $i 2>&1`;
      chomp( $ret_val );
      my $line = getline( $ret_val );
      $line = getlast( $line );
      print STDOUT "$i,$line\n";
   }
}
exit( 0 );

sub getline( $ )
{
   my $str = shift;
   my @lines = split/\n/, $str;
   return( $lines[ 0 ] );
}

sub getlast( $ )
{  
   my $str = shift;
   my @lines = split/ /, $str;
   return( $lines[ -1 ] );
}
