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

        vchar *parse(vchar *str);
    };

    namespace stringMethods {
        nativeFn(substr, interpreter, args, passThisPtr);

        nativeFn(startsWith, interpreter, args, passThisPtr);

        nativeFn(endsWith, interpreter, args, passThisPtr);

        nativeFn(charAt, interpreter, args, passThisPtr);

        nativeFn(fromChar, interpreter, args, passThisPtr);

        nativeFn(length, interpreter, args, passThisPtr);

        nativeFn(rexEqual, interpreter, args, passThisPtr);

        nativeFn(rexNotEqual, interpreter, args, passThisPtr);

        nativeFn(rexAdd, interpreter, args, passThisPtr);

        nativeFn(rexAddAssign, interpreter, args, passThisPtr);

        nativeFn(join, interpreter, args, passThisPtr);

        nativeFn(encode, interpreter, args, passThisPtr);

        nativeFn(trim, interpreter, args, passThisPtr);

        nativeFn(split, interpreter, args, passThisPtr);

        value::cxtObject getMethodsCxt();
    }

    namespace vecMethods {
        namespace iterator {
            nativeFn(next, interpreter, args, passThisPtr);

            value::cxtObject getMethodsCxt(const value::vecObject &container);
        };

        nativeFn(append, interpreter, args, passThisPtr);

        nativeFn(pop, interpreter, args, passThisPtr);

        nativeFn(remove, interpreter, args, passThisPtr);

        nativeFn(removeAll, interpreter, args, passThisPtr);

        nativeFn(rexEqual, interpreter, args, passThisPtr);

        nativeFn(rexNotEqual, interpreter, args, passThisPtr);

        nativeFn(length, interpreter, args, passThisPtr);

        nativeFn(rexIter, interpreter, args, passThisPtr);

        value::cxtObject getMethodsCxt();
    }

    namespace bytesMethods {
        nativeFn(length, interpreter, args, passThisPtr);

        nativeFn(rexEqual, interpreter, args, passThisPtr);

        nativeFn(rexNotEqual, interpreter, args, passThisPtr);

        nativeFn(decode, interpreter, args, passThisPtr);

        nativeFn(concat, interpreter, args, passThisPtr);

        value::cxtObject getMethodsCxt();
    };

    namespace globalMethods {
        namespace charsetsMethods {
            nativeFn(ansiEncoder, interpreter, args, passThisPtr);

            nativeFn(ansiDecoder, interpreter, args, passThisPtr);

            nativeFn(utf8Encoder, interpreter, args, passThisPtr);

            nativeFn(utf8Decoder, interpreter, args, passThisPtr);
        }

        nativeFn(input, interpreter, args, passThisPtr);

        nativeFn(print, interpreter, args, passThisPtr);

        nativeFn(rexRequireMod, interpreter, args, passThisPtr);

        nativeFn(rexRequireNativeMod, interpreter, args, passThisPtr);

        nativeFn(rexRequirePackage, interpreter, args, passThisPtr);

        nativeFn(rexRequire, interpreter, args, passThisPtr);

        nativeFn(format, interpreter, args, passThisPtr);

        value::cxtObject getMethodsCxt();
    }

    namespace threadingMethods {
        nativeFn(start, interpreter, args, passThisPtr);

        nativeFn(wait, interpreter, args, passThisPtr);

        nativeFn(sleep, interpreter, args, passThisPtr);

        value::cxtObject getMethodsCxt();

        value getThreadingModule();
    }
}

#endif //REXSCRIPT_BUILTINMETHODS_HPP
