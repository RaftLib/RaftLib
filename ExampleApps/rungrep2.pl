#!/usr/bin/env perl
use strict;
use warnings;

my $testfile = "~/Downloads/stackoverflow.com-Posts";
#my $testfile = "/dev/shm/PostHistoryShort.xml";
#my $testfile = "~/GIT_RPO/RaftLibDev/ExampleApps/text";
#my $testfile = "~/GIT_RPO/RaftLibDev/ExampleApps/ecoli.txt";
#my $query    = "ACTG";
my $query = "parallel";
#my $query    = "stream\\ processing\\ is\\ cool,\\ so\\ is\\ data-flow\\ programming";
my %programs = ( "rgrep" => "./rgrep" ,"fgrep" => "fgrep", "rgrepless" => "./rgrepless" );
my $runcount = 10;
sub process();

my $parallelgrep = "/usr/bin/time -p parallel --pipe -j 8 /usr/bin/fgrep $query < $testfile > /dev/null 2> /tmp/tmpfile";

my @times;
#for my $key ( keys %programs )
#{
#   for( my $i = 0; $i < $runcount; $i++ )
#   {
#      my $cmd = "/usr/bin/time --portability ".$programs{ $key }." $query $testfile 1> /dev/null 2> /tmp/tmpfile";
#      my $time = `$cmd`;
#      my $time_val = process();
#      my $line = "$key, $testfile, $query, $time_val";
#      push( @times, $line );
#   }
#}

##parallel runs for fgrep
for( my $i = 0; $i < $runcount; $i++ )
{
   print $parallelgrep."\n";
   my $time = `$parallelgrep`;
   my $time_val = process();
   my $line = "gnu parallel bsd fgrep, $testfile, $query, $time_val";
   push( @times, $line );
}

for my $line (@times)
{
   print STDOUT $line."\n";
}


exit( 0 );


sub process()
{
   open TMPFILE, "</tmp/tmpfile" or die "Couldn't open /tmp/tmpfile\n";
   my @lines = <TMPFILE>;
   close( TMPFILE );
   my $line = $lines[ 0 ];
   $line =~ s/\s+/,/g;
   my @fields = split /,/, $line;
   return( $fields[ 1 ] );
}
