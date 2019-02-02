#include <raft>
#include <raftio>
#include <raftalgorithm>
#include <vector>
#include <cstdlib>
#include <iostream>

int
main()
{
    using chunk = raft::filechunk< 512>;
    using fr    = raft::filereader< chunk, false >;
    using search = raft::search< chunk, raft::stdlib >;
    std::vector< raft::match_t > matches;


    const std::string term( "Alice" );
    raft::map m;
    /** pwd is root for cmake's test script **/
    fr   read( "./testsuite/alice.txt" /** ex file **/, 
               (fr::offset_type) term.length(),
               1 );

    search find( term );
    auto we( raft::write_each< raft::match_t >( 
            std::back_inserter( matches ) ) );  
    m += read >> find >> we;
    /** m.exe() is an implicit barrier for completion of execution **/
    m.exe();
    if( matches.size() != 174 /** count from grep **/ )
    {
        return( EXIT_FAILURE );
    }
    std::cout << matches.size() << "\n"; 
   return( EXIT_SUCCESS );
}
