//
// Created by XIaokang00010 on 2022/12/3.
//

#ifndef REXSCRIPT_BUILTINMETHODS_HPP
#define REXSCRIPT_BUILTINMETHODS_HPP

#include "value.hpp"

namespace rex {
    struct formatterTagInfo {
        enum class tagKind : vint {
            vStr,
            vInt,
            vDeci,
            vBool,
            dollar,
        } kind;
        enum class tagBase : vint {
            hex = 16,
            dec = 10,
            oct = 8,
        } base;
        vint precision;

        formatterTagInfo();

        vchar * parse(vchar *str);
    };

    namespace stringMethods {
        value substr(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value startsWith(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value endsWith(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value charAt(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value fromChar(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value length(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value rexEqual(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value rexNotEqual(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value rexAdd(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value rexAddAssign(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value formatter(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value::cxtObject getMethodsCxt();
    }

    namespace vecMethods {
        value::cxtObject getMethodsCxt();
    }

    namespace globalMethods {
        value input(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value print(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value rexImport(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value rexNativeImport(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value format(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr);

        value::cxtObject getMethodsCxt();
    }
}

#endif //REXSCRIPT_BUILTINMETHODS_HPP
