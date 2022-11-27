//
// Created by XIaokang00010 on 2022/11/27.
//

#ifndef REXSCRIPT_VALUE_HPP
#define REXSCRIPT_VALUE_HPP

#include "share/share.hpp"
namespace rex {
    class value {
    public:
        enum class vKind {
            vInt,
            vDeci,
            vBool,
            vStr,
            vObject,
            vUnsafePointer,
            vManagedPointer,
        } kind;

        union vValue {
            vint vInt;
            vdeci vDeci;
            vbool vBool;
            unknownPtr vPtr;

            vValue(vint v);

            vValue(vdeci v);

            vValue(vbool v);

            template<typename T>
            vValue(unsafePtr<T> v) : vPtr(static_cast<unknownPtr>(v)) {}
        } value;

        template<typename T>
        unsafePtr<T> getPtr() {
            return static_cast<unsafePtr<T>>(value);
        }

        const vint & getInt();

        const vdeci & getDeci();

        const vbool & getBool();

        vint &getIntRef();

        vdeci &getDeciRef();

        vbool &getBoolRef();
    };
}

#endif