/**
 * test file from mr-j0nes @github
 * Proof of concept Raftlib
 *
 * Want to have a 3 kernels stream which produces and sends 10 numbers down the stream.
 * The 10 numbers should be created by the first kernel and destroyed by the last.
 *
 */
#include <raft>
#include <iostream>


/**
 * Producer: sends down the stream numbers from 1 to 10
 */
class producer : public raft::kernel
{
private:
    int i = 0;
public:
    producer() : raft::kernel()
    {
        output.addPort< int >( "out" );
    }

    virtual raft::kstatus run()
    {
        /** 
         * Simple generator function, should always
         * return, never keep going in a loop unless
         * you absolutely have to. This enables the
         * runtime to optimize more. Return raft::proceed
         * if there is more output to be sent.
         * when there is no more data to be sent
         * return raft::stop.
         */
        if( i < 1000 )
        {
            /** 
             * output from this "channel" to the
             * next is guaranteed to be ordered, 
             * but only when there is a single 
             * consumer, e.g., you're guaranteed
             * FIFO ordering for that channel.
             */
            output[ "out" ].push( i++ );
            return( raft::proceed );
        }
        else
        {
            return( raft::stop );
        }
    };
};

/**
 * processor - simply takes data in and passes it 
 * through to the next pipeline.
 */
class processor : public raft::kernel
{
public:
    processor() : raft::kernel()
    {
        /** 
         * declare ports, both input and output 
         */
        input.addPort<  int >( "in"  );
        output.addPort< int >( "out" );
    }

    virtual raft::kstatus run()
    {
        /** just a pass through **/
        output[ "out" ].push( input[ "in" ].peek< int >() );
        input[ "in" ].recycle();
        return( raft::proceed );
    }
};

/**
 * Consumer: takes the number from input and dumps it to the console
 */
class consumer : public raft::kernel
{
public:
    consumer() : raft::kernel()
    {
        input.addPort< int >( "in" );
    }

    virtual raft::kstatus run()
    {
        /** print to show we got it **/
        std::cout << input[ "in" ].peek< int >() << "\n";
        input[ "in" ].recycle();
        return( raft::proceed );
    }
};

int  main()
{
    producer    a;
    processor   b;
    consumer    c;

    raft::map m;
    
    m += a >> b >> c;

    m.exe();

    return( EXIT_SUCCESS );
}
