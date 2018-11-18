#include <string>
#include "kernelexception.hpp"



KernelException::KernelException(  const std::string message ) : RaftException( message )
{

}
