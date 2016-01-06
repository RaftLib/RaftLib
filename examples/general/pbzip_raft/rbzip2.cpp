#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <bzlib.h>
#include <cassert>
#include <iostream>
#include <raft>
#include <raftio>

/** 
 * used interface found here: 
 * http://www.bzip.org/1.0.3/html/hl-interface.html 
 */

template < class chunktype > class filewrite : public raft::parallel_k
{
public:
    filewrite( const std::string &&filename ) : raft::parallel_k()
    {
        out_fp = std::fopen( filename.c_str(), "w" );
        if( out_fp == nullptr )
        {   
            std::cerr << "failed to open output file, " << filename << "\n";
            exit( EXIT_FAILURE );
        }  
        addPortTo< chunktype >( input );
    }

    virtual ~filewrite()
    {
        std::fflush( out_fp );
        std::fclose( out_fp );                             
    }

    virtual raft::kstatus run()
    {
        for( auto &port : input )
        {
            if( port.size() > 0 )
            {
                auto &ele( port.template peek< chunktype >() ); 
                if( ele.start_position == curr_position )
                {
                    std::fwrite( ele.buffer, 
                                 1, 
                                 ele.length, 
                                 out_fp );    
                    curr_position += ele.length;
                    port.recycle( ele, 1 );
                }
            }
            else
            {
                port.unpeek();
            }
        }
        return( raft::proceed );
    }

private:
    FILE *out_fp              = nullptr;
    std::size_t curr_position = 0;
};

template < class chunktype > class compress : public raft::kernel
{
public:
    compress( const int blocksize, 
              const int verbosity,
              const int workfactor ) : raft::kernel(),
                                       blocksize( blocksize ),
                                       verbosity( verbosity ),
                                       workfactor( workfactor )
    {
        input.addPort< chunktype >( "in" );
        output.addPort< chunktype >( "out" );
    }

    virtual ~compress() = default;

    virtual raft::kstatus run()
    {
        auto &in_ele( input[ "in" ].template peek< chunktype >() );
        auto &out_ele(  output[ "out" ].template allocate< chunktype >() );
        unsigned int length_out( 0 );
        if( BZ2_bzBuffToBuffCompress( out_ele.buffer,
                                      &length_out,
                                      in_ele.buffer,
                                      in_ele.length,
                                      blocksize,
                                      verbosity,
                                      workfactor ) == BZ_OK )
        {
            
            out_ele.length = length_out;
            output[ "out" ].send();
            input[ "in" ].recycle( in_ele, 1 );
        }
        else
        {
            /** we need to fail here **/
            output[ "out" ].deallocate();
            input[ "in" ].unpeek();
            exit( EXIT_FAILURE );
        }
        return( raft::proceed );
    }
              
private:
    const int blocksize;
    const int verbosity;
    const int workfactor;
};

int
main( int argc, char **argv )
{
    using chunk_t = raft::filechunk< 16384 >;
    using fr_t    = raft::filereader< chunk_t, false >;
    using fw_t    = filewrite< chunk_t >;
    using comp    = compress< chunk_t >;

    int blocksize ( 9 );
    int verbosity ( 0 );
    int workfactor( 30 );
   

    fr_t reader( argv[ 1 ] );
    comp c( blocksize, verbosity, workfactor );
    fw_t writer( argv[ 2 ] );
    
    raft::map m;
    m += reader >> c;
    m += c >> writer;

    m.exe();
    return( EXIT_SUCCESS );
}
