// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#include <luabind/lua_include.hpp>

#include <luabind/luabind.hpp>
#include <luabind/detail/implicit_cast.hpp>

namespace luabind { namespace detail {

    // returns -1 if there exists no implicit cast from the given class_rep
    // to T. If there exists an implicit cast to T, the number of steps times
    // 2 is returned and pointer_offset is filled with the number of bytes 
    // the pointer have to be offseted to perform the cast the reason why we
    // return the number of cast-steps times two, instead of just the number 
    // of steps is to be consistent with the function matchers. They have to 
    // give one matching-point extra to match const functions. There may be
    // two functions that match their parameters exactly, but there is one 
    // const function and one non-const function, then (if the this-pointer
    // is non-const) both functions will match. To avoid amiguaties, the const
    // version will get one penalty point to make the match-selector select
    // the non-const version. If the this-pointer is const, there's no 
    // problem, since the non-const function will not match at all.
    
    namespace
    {
        int find_implicit_cast(
            class_rep const* crep
          , LUABIND_TYPE_INFO const& type_id
          , int& pointer_offset)
        {
            int offset = 0;
            if (LUABIND_TYPE_INFO_EQUAL(crep->type(), type_id)) return 0;

            for (std::vector<class_rep::base_info>::const_iterator i = 
                crep->bases().begin(); i != crep->bases().end(); ++i)
            {
                int steps = find_implicit_cast(i->base, type_id, offset);
                pointer_offset = offset + i->pointer_offset;
                if (steps >= 0) return steps + 2;
            }
            return -1;
        }
    }

    // Finding the cast compares type names all the way up the bases, so what was found is kept in the class_rep for each type.
    // Finding it writes the pointer offset for every base it tries, so the cached result writes it too, unless the class is the type itself or has no bases.
    int implicit_cast(
        class_rep const* crep
      , LUABIND_TYPE_INFO const& type_id
      , int& pointer_offset)
    {
        class_rep::implicit_cast_results& results = crep->implicit_casts();
        class_rep::implicit_cast_results::const_iterator found = results.find(type_id);
        if (found == results.end())
        {
            class_rep::implicit_cast_result result;
            result.pointer_offset = 0;
            result.steps = find_implicit_cast(crep, type_id, result.pointer_offset);
            result.writes_pointer_offset = !(LUABIND_TYPE_INFO_EQUAL(crep->type(), type_id)) && !crep->bases().empty();
            found = results.insert(std::make_pair(type_id, result)).first;
        }
        if (found->second.writes_pointer_offset) pointer_offset = found->second.pointer_offset;
        return found->second.steps;
    }
}}

