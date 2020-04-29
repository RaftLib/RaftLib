template < std::size_t N > struct foo
{
   foo() = default;
   ~foo() = default;
  
   constexpr foo( const foo &other )
   {
        using index_type = std::remove_const_t< decltype( N ) >;
        for( index_type i( 0 ); i < N; i++ )
        {
            pad[ i ] = other.pad[ i ];
        }
   }

   constexpr foo& operator = ( const foo &other )
   {
        using index_type = std::remove_const_t< decltype( N ) >;
        for( index_type i( 0 ); i < N; i++ )
        {
            pad[ i ] = other.pad[ i ];
        }
        return( *this );
   }
    

   int length       = N;
   int pad[ N ]     = { 0 };
};
