/**
 * units.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Aug 25 12:19:26 2014
 * 
 * Copyright 2014 Jonathan Beard
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
#ifndef _UNITS_HPP_
#define _UNITS_HPP_  1

/** unit enum used throughout **/
enum Unit : std::size_t { Byte = 0, KB, MB, GB, TB, N };

/** constants for use later **/
static constexpr std::array< double, 
                  Unit::N > unit_conversion
                      = {{ 1              /** bytes **/,
                           0.000976562    /** kilobytes **/,
                           9.53674e-7     /** megabytes **/, 
                           9.31323e-10    /** gigabytes **/,
                           9.09495e-13    /** terabytes **/ }};

static constexpr std::array<  const char[3] , 
                  Unit::N > unit_prints
                         = {{ "B", "KB", "MB", "GB", "TB" }};

#endif /* END _UNITS_HPP_ */
