#include "demangle.hpp"

#if defined( __clang__ ) && defined( __has_include )
# if __has_include(<cxxabi.h>)
# include <cxxabi.h>
# endif
#elif defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
# include <cxxabi.h>
#endif


namespace raft {
#if defined( _CXXABI_H )
std::string demangle(const char *name) {
  int status = -4;
  char *res = abi::__cxa_demangle(name, NULL, NULL, &status);
  const char *const demangled_name = (status == 0) ? res : name;
  std::string val(demangled_name);
  std::free(res);
  return val;
}
#else
std::string demangle(const char *name) { return std::string(name); }
#endif
} // namespace raft
