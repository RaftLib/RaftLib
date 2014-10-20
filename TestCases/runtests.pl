#!/usr/bin/env perl
use strict;
use warnings;

sub executeCommand( $ );

my  @tests = qw( allocate dynallocate portTypeException );
foreach my $test( @tests )
{
   my $testStatus = 1;
   my ($status, $output ) = executeCommand( $test );
   if( $status ne "0" )
   {
      my $errfile = $test."_err_expect.txt";
      my $wholeerrtext = do{
         local $/ = undef;
         open my $errfh, "<",$errfile  or die "couldn't open $errfile";
         <$errfh>;
      };
      if( !( $wholeerrtext eq $status ) )
      {
         $testStatus = 0;
      }
   }
   print "OUTPUT: $output\n";
   if( length( $output ) > 0 )
   {
      my $file = $test."_out_expect.txt";
      my $wholetext = do{
         local $/ = undef;
         open my $fh, "<",$file  or die "couldn't open $file";
         <$fh>;
      };
      if( !( $wholetext eq $output ) )
      {
         $testStatus = 0;
      }
   }
   print "Test: $test has ".( $testStatus == 1 ? "PASSED" : "FAILED" )."\n";
}

sub executeCommand( $ )
{
   my $command = shift;
   $command = "./".$command;
   ($? >> 8, $_ = qx{$command 2>&1});
}
