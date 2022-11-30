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
            vNull = 0b000000001,
            vInt = 0b000000010,
            vDeci = 0b000000100,
            vBool = 0b000001000,
            vStr = 0b000010000,
            vObject = 0b000100000,
            vVec = 0b001000000,
            vRef = 0b010000000,
        } kind;

        union vValue {
            vint vInt;
            vdeci vDeci;
            vbool vBool;
            unsafePtr<unknownPtr> vPtr;

            vValue();

            vValue(vint v);

            vValue(vdeci v);

            vValue(vbool v);

            template<typename T1>
            vValue(unsafePtr<T1> v) : vPtr(static_cast<unsafePtr<unknownPtr>>(v)) {}
        } val;

        map<vstr, value> object;

        template<typename T>
        T& getPtr() {
            return static_cast<unsafePtr<T>>(val.vPtr);
        }

        const vint & getInt();

        const vdeci & getDeci();

        const vbool & getBool();

        vint &getIntRef();

        vdeci &getDeciRef();

        vbool &getBoolRef();

        value();

        value(vKind k, const vValue &v);
    };
}

#endif