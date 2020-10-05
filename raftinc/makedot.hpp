/**
 * makedot.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Oct  4 09:15:09 2020
 * 
 * Copyright 2020 Jonathan Beard
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
#ifndef MAKEDOT_HPP
#define MAKEDOT_HPP  1
#include <ostream>
#include <sstream>
#include "kernelkeeper.tcc"

namespace raft
{
    
    /** base template **/
    template < class... TS > struct field_helper{};
    
    /** recursive template base case for fields **/
    template <> struct field_helper<>
    {
         static std::string get_csv() { return( "" ); }
    };
    
    /** recursive template main case for fields **/
    template < class T, class... TS > struct field_helper< T, TS... >
    {
        /**
         * get_csv - recursive calls to this function build a 
         * string with whatever type was given in the initial
         * invocation....they all have to be teh same time, the
         * string as the name implies is comma separated. 
         */
        static std::string get_csv( const T &&str, TS&&... str_list )
        {
            std::stringstream ss;
            ss << str;
            return( std::move< std::string >( 
                        ss.str() + 
                        (sizeof...( TS ) == 0 ? "" : ", " ) +
                        field_helper< TS... >::get_csv( 
                            std::forward< TS >( str_list )... 
                        ) /** end recursive call **/
                     ) /** end move **/
                   ) /** end ret **/;
        }
    };

//declare vs. include here to avoid header conflicts
class map;

class make_dot
{
public:
    
    template < class T = raft::make_dot > static 
        void run( std::ostream &stream, raft::map &map )
    {
        T   dot_maker( map );
        dot_maker._run( stream );
        return;
    }

protected:
    make_dot( raft::map &map );

    virtual ~make_dot() = default;

    /** 
     * call this to kick off graph construction,
     * will dump the file to the stream with 
     * a proper "digraph" format, override this 
     * function or one of the "protected" functions
     * below to redefine this behavior for sub-classes
     * if you want differing behavior. 
     */
    virtual void _run( std::ostream &stream );

    virtual void generate_preamble( std::ostream &stream );
    virtual void generate_vertex_list( std::ostream &stream );
    virtual void generate_edge_list( std::ostream &stream );
    virtual void generate_close( std::ostream &stream );
    
    
    
     template< class... TS > static std::string generate_field( 
        const std::string &&name, TS&&... str_list )
    {
        return( std::move< std::string >( name + "=\"" + field_helper< TS... >::get_csv( std::forward< TS >( str_list )... )  +"\"" ) );
    }

    int height = 10;
    int width  = 10;
private:
    /** 
     * borrowing the list of all source kernels,
     * remember, if you want updates over time
     * of what the graph looks like, this will need
     * to be used in a lock-safe manner. For multi-
     * process applications we'll also need similar 
     * functionality at the ``oar'' level in order
     * to propagate the graph. 
     */
    kernelkeeper &all_kernels; 
    kernelkeeper &source_kernels; 
};

} /** end namespace raft **/

#endif /* END MAKEDOT_HPP */
