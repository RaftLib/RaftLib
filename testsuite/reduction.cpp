#include <raft>

struct reduction_msg
{
    int *pcnt;
    int cnt;
};

class reduce : public raft::parallel_k
{
public:
    reduce( int ninputs ) : raft::parallel_k()
    {
        for( int i( 0 ); ninputs > i; ++i )
        {
            addPortTo< struct reduction_msg >( input );
        }
    }
    virtual raft::kstatus run()
    {
        for( auto &port : input )
        {
            if( 0 < port.size() )
            {
                auto &msg( port.template peek< struct reduction_msg >() );
                *( msg.pcnt ) += msg.cnt;
                port.recycle( 1 );
            }
        }
        return raft::proceed;
    }
};

class count : public raft::kernel
{
public:
    count( int *cnts ) : raft::kernel(), pcnts( cnts )
    {
        input.addPort< int >( "input" );
        output.addPort< struct reduction_msg >( "output" );
    }
    count( const count &other ) : raft::kernel(), pcnts( other.pcnts )
    {
        input.addPort< int >( "input" );
        output.addPort< struct reduction_msg >( "output" );
    }
    ~count() = default;
    virtual raft::kstatus run()
    {
        int idx;
        input["input"].pop< int >( idx );
        while( idx ) {
            output["output"].push< struct reduction_msg >(
                    { &pcnts[ idx ], idx ^ 0x123 } );
            idx >>= 1;
        }
        return raft::proceed;
    }
    CLONE(); // enable cloning
private:
    int *pcnts;
};

class workset : public raft::parallel_k
{
public:
    workset( int noutputs, int m ) : raft::parallel_k(), idx_max( m )
    {
        for( int i( 0 ); noutputs > i; ++i )
        {
            addPortTo< int >( output );
        }
        idx = 0;
    }
    virtual raft::kstatus run()
    {
        for( auto &port : output )
        {
            if( port.space_avail() )
            {
                port.template push< int >( idx );
                if( idx_max <= ++idx )
                {
                    return raft::stop;
                }
            }
        }
        return raft::proceed;
    }
private:
    int idx_max, idx;
};

int main( int argc, char **argv )
{
   int arg_cnt( 1000 );
   int arg_run( 60 );
   if( argc >= 2 )
   {
      arg_cnt = atoi( argv[ 1 ] );
   }
   if( argc >= 3 )
   {
      arg_run = atoi( argv[ 2 ] );
   }
   int *cnts = new int[ arg_cnt ];

   for( int i( 0 ); arg_run > i; ++i )
   {
       workset workset_k( 4, arg_cnt );
       reduce reduce_k( 4 );
       count count_k( cnts );

       raft::map m;
       m += workset_k <= count_k >= reduce_k;

       m.exe();
   }

   return( EXIT_SUCCESS );
}
