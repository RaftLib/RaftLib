/**
 * kernel_wrapper.hpp - this class is designed to be used only by the kernel.hpp
 * make kernel function. It's supposed to be a wrapper class for the kernel type
 * and sub-types. It should ahve a destructor that destroys the wrapped pointer
 * on scope exit if it hasn't already been deallocated (attempting to be extra 
 * safe). Its main function is to provide a syntax marker for the operator >>
 * overloads in the kpair.hpp file so that the compiler knows exactly what to 
 * link to. The other reason for this class's existence is b/c we can't really 
 * use the move syntax without a complicated copy. The kernel itself cannot live
 * on the stack, it must live in the heap or elsewhere.
 *
 * @author: Jonathan Beard
 * @version: Thu Sep  1 16:12:43 2016
 * 
 * Copyright 2016 Jonathan Beard
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
#ifndef _KERNEL_WRAPPER_HPP_
#define _KERNEL_WRAPPER_HPP_  1
#include <memory>

namespace raft
{
    class kernel;
    using kernel_wrapper = std::shared_ptr< raft::kernel >;
}

#endif /* END _KERNEL_WRAPPER_HPP_ */
