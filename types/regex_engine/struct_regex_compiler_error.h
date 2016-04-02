#ifndef TYPES_REGEX_ENGINE_struct_regex_compiler_error_H_
#define TYPES_REGEX_ENGINE_struct_regex_compiler_error_H_
/*
    Copyright 2016 Robert Elder Software Inc.
    
    Licensed under the Apache License, Version 2.0 (the "License"); you may not 
    use this file except in compliance with the License.  You may obtain a copy 
    of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software 
    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT 
    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the 
    License for the specific language governing permissions and limitations 
    under the License.
*/

#ifndef TYPES_REGEX_ENGINE_enum_regex_compiler_error_type_H_
#include "enum_regex_compiler_error_type.h"
#endif

struct regex_compiler_error;
struct regex_compiler_error{
	struct regex_compiler_error * next;
	enum regex_compiler_error_type type;
	unsigned int error_raised;
	unsigned int character_index_of_error;
	unsigned int pad;
};

#endif