#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <bzlib.h>
#include <cassert>
#include <iostream>
#include <raft>
#include <raftio>
#include <cmd>
#include <type_traits>

/**
 * used interface found here:
 * http://www.bzip.org/1.0.3/html/hl-interface.html
 */

template < class chunktype > class filewrite : public raft::parallel_k
{
public:
    filewrite( const std::string filename,
               const std::size_t n_output_ports = 1 ) : raft::parallel_k()
    {
        out_fp = std::fopen( filename.c_str(), "wb" );
        if( out_fp == nullptr )
        {
            std::cerr << "failed to open output file, " << filename << "\n";
            exit( EXIT_FAILURE );
        }

        using index_type = std::remove_const_t<decltype(n_output_ports)>;
        for( index_type i( 0 ); i < n_output_ports; i++ )
        {
            addPortTo< chunktype >( input );
        }
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
                if( ele.index == curr_position )
                {
                    std::fwrite( ele.buffer,
                                 1,
                                 ele.length,
                                 out_fp );
                    curr_position++;
                    port.recycle( 1 );
                }
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

    /** need a copy constructor for cloning **/
    compress( const compress &other ) : raft::kernel(),
                                        blocksize( other.blocksize ),
                                        verbosity( other.verbosity ),
                                        workfactor( other.workfactor )
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
            out_ele.index  = in_ele.index;
            output[ "out" ].send();
            input[ "in" ].recycle( 1 );
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
                break;
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

    /** enable cloning **/
    CLONE();

private:
    const int blocksize;
    const int verbosity;
    const int workfactor;
};

int
main( int argc, char **argv )
{
    const static auto chunksize( 65536 );
    using chunk_t = raft::filechunk< chunksize >;
    using fr_t    = raft::filereader< chunk_t, false >;
    using fw_t    = filewrite< chunk_t >;
    using comp    = compress< chunk_t >;

    /** variables to set below **/
    bool help( false );
    int blocksize ( 9 );
    int verbosity ( 0 );
    int workfactor( 250 );
    std::string inputfile( "" );
    std::string outputfile( "" );

    std::int64_t  num_threads( 1 );

    CmdArgs cmdargs( argv[ 0 ] /** prog name  **/,
                     std::cout /** std stream **/,
                     std::cerr /** err stream **/ );

    /** set options **/
    cmdargs.addOption( new Option< bool >( help,
                                           "-h",
                                           "print this message" ) );
    cmdargs.addOption( new Option< int >( blocksize,
                                          "-b",
                                          "set block size to 100k .. 900k" ) );
    cmdargs.addOption( new Option< int >( workfactor,
                                          "-e",
                                          "effort" ) );
    cmdargs.addOption( new Option< std::string >( inputfile,
                                                  "-i",
                                                  "input file",
                                                  true /** required **/ ) );

    cmdargs.addOption( new Option< std::string >( outputfile,
                                                  "-o",
                                                  "output file",
                                                  true /** required **/ ) );

    cmdargs.addOption( new Option< std::int64_t  >( num_threads,
                                                    "-th",
                                                    "number of worker threads" ) );

    /** process args **/
    cmdargs.processArgs( argc, argv );
    if( help || ! cmdargs.allMandatorySet() )
    {
        cmdargs.printArgs();
        exit( EXIT_SUCCESS );
    }

    /** declare kernels **/
    fr_t reader( inputfile,
                 0, /** no offset needed **/
                 num_threads /** manually set threads for b-marking **/ );
    fw_t writer( outputfile,
                 num_threads /** manually set threads for b-marking **/ );
    comp c( blocksize,
            verbosity,
            workfactor );

    /** set-up map **/
    raft::map m;

    /**
     * detect # output ports from reader,
     * duplicate c that #, assign each
     * output port to the input ports in
     * writer
     */
    m += reader <= c >= writer;

    m.exe();
    return( EXIT_SUCCESS );
}
