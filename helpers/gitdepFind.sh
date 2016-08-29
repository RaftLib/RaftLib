git submodule status | perl -e 'while( <> ){ if( $_ =~ /(?<=git-dep\/)\w+(?=\s+)/ ){ chomp( $& ); print $&." "; } }'
