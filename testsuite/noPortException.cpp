/**
 * Example from Issue #43, seg fault when no port is 
 * available to connect to. Should throw an exception
 */

#include <raft>
#include <raftio>


struct big_t
{
    int i;
    std::uintptr_t start;
#ifdef ZEROCPY
    char padding[ 100 ];
#else
    char padding[ 32 ];
#endif
};



/**
 * Producer: sends down the stream numbers from 1 to 10
 */
class A : public raft::kernel
{
private:
    int i   = 0;

public:
    A() : raft::kernel()
    {
        output.addPort< big_t >("out");
    }
    
    IMPL_CLONE();

    virtual raft::kstatus run()
    {
        i++;

        if ( i <= 10 ) 
        {
            auto &c( output["out"].allocate< big_t >() );
            c.i = i;
            c.start = reinterpret_cast< std::uintptr_t >( &(c.i) );
            output["out"].send();
        }
        else
        {   
            return (raft::stop);
        }

        return (raft::proceed);
    };
};

/**
 * Consumer: takes the number from input and dumps it to the console
 */
class C : public raft::kernel
{
public:
    C() : raft::kernel()
    {
        //input.addPort< big_t >("in");
    }
    
    IMPL_CLONE();

    virtual raft::kstatus run()
    {
        auto &a( input[ "in" ].peek< big_t >() );
        std::cout << std::dec << a.i << " - " << std::hex << a.start << " - " << std::hex <<  
            reinterpret_cast< std::uintptr_t >( &a.i ) << "\n";
        input[ "in" ].recycle(1);

        input[ "in" ].recycle(1);
        return (raft::proceed);
    }
};

int main()
{
    A a;
    C c;
    raft::map m;
    try
    {
        m += a >> c;
    }
    catch( PortNotFoundException &ex )
    {
        std::cerr << ex.what() << "\n";
        /** success for test case at least **/
        exit( EXIT_SUCCESS );
    }
    m.exe();
    return( EXIT_FAILURE );
}
