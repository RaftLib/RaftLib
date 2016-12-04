git submodule status | perl -e 'while( <> ){ if( $_ =~ /(?<=boost-dep\/)\w+(?=\s+)/ ){ chomp( $& ); print $&." "; } }'
