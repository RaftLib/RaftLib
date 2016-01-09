#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <raftrandom>


/**
 * Sum template extends the raft::kernel base class,
 * and provides an implementation of the run function
 * which is run by the RaftLib scheduler.
 */
template < typename T > class Sum : public raft::kernel
{
public:
   Sum() : raft::kernel()
   {
      /** define ports **/
      input.addPort<  T > ( "input_a", "input_b" );
      output.addPort< T  >( "sum"     );
   }
   
   virtual raft::kstatus run()
   {
      T a,b; 
      /** ports are accessed via names defined above **/
      input[ "input_a" ].pop( a );
      input[ "input_b" ].pop( b );
      /** allocate mem directly on queue **/
      auto c( output[ "sum" ].template allocate_s< T >() );
      (*c) = a + b;
      /** 
       * mem for C pushed to queue on scope exit, proceed 
       * signals the run-time to continue 
       */
      return( raft::proceed );
   }

};


int
main( int argc, char **argv )
{
   const auto count( 1000 );
   using type_t = std::int64_t;
   /** 
    * random_variate is a threaded wrapper around the C++ random number gen **/
   using gen = raft::random_variate< std::default_random_engine,
                                     std::uniform_int_distribution,
                                     type_t >;
                               
   using sum    = Sum< type_t >;
   using print  = raft::print< type_t, '\n' >;

   gen a( count ), b( count );
   sum s;
   print p;
   raft::map m;
   /** link the only output port of a to the "input_a" port of s **/
   m += a >> s[ "input_a" ];
   /** link the only output port of b to the "input_b" port of s **/
   m += b >> s[ "input_b" ];
   /** take the only output port of s and link it to the only input port of p **/
   m += s >> p;
   /** 
    * NOTE: this will be going away soon, 
    * to be called on scope exit, an explicit
    * barrier call will enable integration with
    * sequential code.
    */
   m.exe();
   return( EXIT_SUCCESS );
}
