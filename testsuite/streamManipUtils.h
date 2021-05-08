#include <raft>
#include <cstddef>
#include <cstdint>
#include <string>
#include <cstdlib>
#include "foodef.tcc"


using obj_t = foo< 80 >;

class start : public raft::kernel
{
public:
    start() : raft::kernel()
    {
        output.addPort< obj_t >( "y" );
    }

    virtual ~start() = default;
    
    virtual raft::kstatus run()
    {
        auto &mem( output[ "y" ].allocate< obj_t >() );
        for( auto i( 0 ); i < mem.length; i++ )
        {
            mem.pad[ i ] = static_cast< int >( counter );
        }
        output[ "y" ].send();
        counter++;
        if( counter == 200 )
        {
            return( raft::stop );
        }
        return( raft::proceed );
    }

private:
    std::size_t counter = 0;
};


class middle : public raft::kernel
{
public:
    middle() : raft::kernel()
    {
        input.addPort< obj_t >( "x" );
        output.addPort< obj_t >( "y" );
    }
    virtual raft::kstatus run()
    {
        auto &val( input[ "x" ].peek< obj_t >() );
        output[ "y" ].push( val );
        input[ "x" ].unpeek();
        input[ "x" ].recycle( 1 );
        return( raft::proceed );
    }
};


class last : public raft::kernel
{
public:
    last() : raft::kernel()
    {
        input.addPort< obj_t >( "x" );
    }
    
    virtual ~last() = default;
    
    virtual raft::kstatus run()
    {
        obj_t mem;
        input[ "x" ].pop( mem );
        /** Jan 2016 - otherwise end up with a signed/unsigned compare w/auto **/
        using index_type = std::remove_const_t< decltype( mem.length ) >;
        for( index_type i( 0 ); i < mem.length; i++ )
        {   
            //will fail if we've messed something up
            if( static_cast< std::size_t >( mem.pad[ i ] ) != counter )
            {
                std::cerr << "failed test\n"; 
                exit( EXIT_FAILURE );
            }
        }
        counter++;
        return( raft::proceed );
    }

private:
    std::size_t counter = 0;
};

