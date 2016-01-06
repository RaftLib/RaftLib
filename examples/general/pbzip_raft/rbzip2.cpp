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
        out_fp = std::fopen( filename.c_str(), "wb" );
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
                //if( ele.start_position == curr_position )
                //{
                    std::fwrite( ele.buffer, 
                                 1, 
                                 ele.length, 
                                 out_fp );    
                    curr_position += ele.length;
                    port.recycle( ele, 1 );
                //}
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
        unsigned int length_out( chunktype::getChunkSize() );
        const auto ret_val( BZ2_bzBuffToBuffCompress( out_ele.buffer,
                                      &length_out,
                                      in_ele.buffer,
                                      in_ele.length,
                                      blocksize,
                                      verbosity,
                                      workfactor ) );
        if( ret_val == BZ_OK )
        {
            
            out_ele.length = length_out;
            output[ "out" ].send();
            input[ "in" ].recycle( in_ele, 1 );
        }
        else
        {
            switch( ret_val )
            {
                case BZ_CONFIG_ERROR:
                {
                    std::cerr << "if the library has been mis-compiled\n";
                }
                break;
                case BZ_PARAM_ERROR:
                {
                    std::cerr << "if dest is NULL or destLen is NULL\n";
                    std::cerr << "or blockSize100k < 1 or blockSize100k > 9\n";
                    std::cerr << "or verbosity < 0 or verbosity > 4\n";
                    std::cerr << "or workFactor < 0 or workFactor > 250\n";
                }
                break;
                case BZ_MEM_ERROR:
                {
                    std::cerr << "if insufficient memory is available\n";
                }
                break;
                case BZ_OUTBUFF_FULL:
                {
                    std::cerr << "if the size of the compressed data exceeds *destLen\n";
                }
                default:
                {
                    std::cerr << "undefined error\n";
                }
            }
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
    const static auto chunksize( 4096 );
    using chunk_t = raft::filechunk< chunksize >;
    using fr_t    = raft::filereader< chunk_t, false >;
    using fw_t    = filewrite< chunk_t >;
    using comp    = compress< chunk_t >;

    int blocksize ( 3 );
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
