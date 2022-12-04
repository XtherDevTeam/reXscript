//
// Created by XIaokang00010 on 2022/12/3.
//

#ifndef REXSCRIPT_BUILTINMETHODS_HPP
#define REXSCRIPT_BUILTINMETHODS_HPP

#include "value.hpp"

namespace rex {
    namespace stringMethods {
        value substr(void* interpreter, vec<value> args, managedPtr<value> passThisPtr);

        value startsWith(void* interpreter, vec<value> args, managedPtr<value> passThisPtr);

        value endsWith(void* interpreter, vec<value> args, managedPtr<value> passThisPtr);

        value charAt(void* interpreter, vec<value> args, managedPtr<value> passThisPtr);

        value fromChar(void* interpreter, vec<value> args, managedPtr<value> passThisPtr);

        value length(void* interpreter, vec<value> args, managedPtr<value> passThisPtr);

        value::cxtObject getMethodsCxt();
    }

    namespace vecMethods {
        value::cxtObject getMethodsCxt();
    }
}

#endif //REXSCRIPT_BUILTINMETHODS_HPP
