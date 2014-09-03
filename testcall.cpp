#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <map>
#include <ostream>
#include <typeinfo>
#include <utility>

class PrintBase
{
public:
virtual std::ostream& print( std::ostream &in ) = 0;
};

template <typename T > class Print : public PrintBase
{
public:
   Print()          = default;
   virtual ~Print() = default;

   virtual std::ostream& print( std::ostream &in )
   {
      in << typeid( T ).name();
      return( in );
   }
};



template <typename a, typename b >class Test
{
public:
   Test()
   {
      themap.insert( std::make_pair( "int", new Print< a >() ) );
      themap.insert( std::make_pair( "float", new Print< b >() ) );;
   }

   PrintBase& operator[]( const char *str )
   {
      return( *themap[ str ] );
   }


private:
   std::map< std::string, PrintBase* > themap; 
};

int
main( int argc, char **argv )
{
   Test<int,float> t;
   t[ "int" ].print( std::cout ) << "\n";
   t[ "float" ].print( std::cout ) << "\n";
   return( EXIT_SUCCESS );
}
