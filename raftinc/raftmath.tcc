/**
 * raftmath.tcc -
 * @author: Jonathan Beard
 * @version: Tue Apr 28 13:48:37 2015
 *
 * Copyright 2015 Jonathan Beard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _RAFTMATH_TCC_
#define _RAFTMATH_TCC_  1

namespace raft
{


/** START recursive templates for sum **/
template < class RETTYPE, class... PORTS > struct sum_helper{};
/** dummy template to end the recursion **/
template < class RETTYPE > struct sum_helper< RETTYPE >
{
   constexpr static RETTYPE sum()
   {
      return( std::move< RETTYPE >( static_cast< RETTYPE >( 0 ) ) );
   }
};
/** helper struct, for recursive template **/
template < class RETTYPE, class PORT, class... PORTS >
   struct sum_helper< RETTYPE, PORT, PORTS... >
{
   static RETTYPE sum( PORT &&port, PORTS&&... ports )
   {
      RETTYPE val;
      port.pop( val );
      return(  std::move< RETTYPE >(
                  val + sum_helper< RETTYPE, PORTS... >::sum(
                     std::forward< PORTS >( ports )... ) ) );
   }
};
/**
 * sum - takes a set of ports in, pops the head off the FIFOs
 * and adds them, moving the value to the return which is
 * well the return.
 */
template < typename RETTYPE,
           class... PORTS,
        typename std::enable_if< std::is_arithmetic< RETTYPE >::value >::type* = nullptr
           > static RETTYPE sum(  PORTS&&... ports )
{
   return( std::move< RETTYPE >( sum_helper< RETTYPE, PORTS... >::sum(
               std::forward< PORTS >( ports )... ) ) );
}

/** START recursive templates for mult **/
template < class RETTYPE, class... PORTS > struct mult_helper{};
/** dummy template to end the recursion **/
template < class RETTYPE > struct mult_helper< RETTYPE >
{
   constexpr static RETTYPE mult()
   {
      return( std::move< RETTYPE >( static_cast< RETTYPE >( 1 ) ) );
   }
};
/** helper struct, for recursive template **/
template < class RETTYPE, class PORT, class... PORTS >
   struct mult_helper< RETTYPE, PORT, PORTS... >
{
   static RETTYPE mult( PORT &&port, PORTS&&... ports )
   {
      RETTYPE val;
      port.pop( val );
      return(  std::move< RETTYPE >(
                  val * mult_helper< RETTYPE, PORTS... >::mult(
                     std::forward< PORTS >( ports )... ) ) );
   }
};
/**
 * mult - takes a set of ports in, pops the head off the FIFOs
 * and adds them, moving the value to the return which is
 * well the return.
 */
template < typename RETTYPE,
           class... PORTS,
        typename std::enable_if< std::is_arithmetic< RETTYPE >::value >::type* = nullptr
           > static RETTYPE mult(  PORTS&&... ports )
{
   return( std::move< RETTYPE >( mult_helper< RETTYPE, PORTS... >::mult(
            std::forward< PORTS >( ports )... ) ) );
}


} /** end namespace raft **/

#endif /* END _RAFTMATH_TCC_ */
