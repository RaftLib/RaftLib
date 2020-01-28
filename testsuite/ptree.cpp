/** undefine private if defined **/
#ifdef  private
#undef  private
#endif 

/** define private as public to expose struct **/
#define private public 
#include "parsetree.hpp"

/** get rid of private define, not needed for rest of headers **/
#undef private

#include "parsetreedefs.hpp"

#include <cstdlib>


int main()
{
    /** 
     * test new overload functions:
     * get_lhs -> should return ptree.second
     * get_rhs -> should return ptree.first
     * 
     * post condition: 
     * return should also enable return of a nullptr
     * wrapped in a unique pointer. 
     * 
     * pre-condition: 
     * 1) default state of the parse_tree should be to contain 
     * a null lhs pointer and null rhs pointer. 
     * 
     * on destruction: 
     * all unique pointers decremented
     */
    
    /** test constructors **/
    ptree_t< frontier_t > parse_tree1;
    ptree_t< frontier_t > parse_tree2( 
     
    return( EXIT_SUCCESS );
}
