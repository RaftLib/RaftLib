#!/usr/bin/env perl
use strict;
use warnings;

##
# added __linux check since well, this only works
# for systems with /proc fs. Need to check Win
# for NUMA control. Mac OS doesn't give find grained
# NUMA control even for the few of their systems that
# have it...
# TODO, check for hetereogeneous mem controls
##
if( defined $ENV{ "__linux" } )
{
    my $cmdstr =  "grep -oP \"(?<=Node\\s)\\d\" /proc/zoneinfo";
    my $nodelist = `$cmdstr`;
    chomp( $nodelist );
    my @arr = split/\n/,$nodelist;
    my $first = shift( @arr );
    foreach my $val ( @arr )
    {
        if( $val != $first )
        {
            print( "1" );
            exit( 0 );
        }
    }
    `touch foo`;
    exit( 0 );
}
print( "0" );
exit( 0 );
