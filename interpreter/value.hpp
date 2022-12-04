//
// Created by XIaokang00010 on 2022/12/3.
//

#ifndef REXSCRIPT_VALUE_HPP
#define REXSCRIPT_VALUE_HPP

#include "frontend/ast.hpp"
#include "share/share.hpp"

namespace rex {
    class value {
    public:
        using vecObject = vec<managedPtr<value>>;
        using cxtObject = map<vstr, managedPtr<value>>;
        using nativeFuncPtr = std::function<value(void*, vec<value>, managedPtr<value>)>;

        struct funcObject {
            vec<vstr> argsName;
            AST code;

            funcObject();

            funcObject(const vec<vstr> &argsName, AST code);
        };

        struct lambdaObject {
            managedPtr<value> outerCxt;
            funcObject func;

            lambdaObject();

            lambdaObject(managedPtr<value> outerCxt, funcObject func);
        };

        enum class vKind : uint16_t {
            vNull,
            vInt,
            vDeci,
            vBool,
            vStr,
            vVec,
            vObject,
            vFunc,
            vLambda,
            vNativeFuncPtr,
            vRef,
        } kind;

        union vValue {
            vint vInt;
            vdeci vDeci;
            vbool vBool;

            vValue();

            vValue(vint v);

            vValue(vdeci v);

            vValue(vbool v);
        } basicValue;

        // objects are saved here.
        managedPtr<value> refObj;
        managedPtr<vstr> strObj;
        managedPtr<vecObject> vecObj;
        managedPtr<funcObject> funcObj;
        managedPtr<lambdaObject> lambdaObj;
        managedPtr<nativeFuncPtr> nativeFuncObj;

        // members
        cxtObject members;

        bool isRef();

        // make a deep copy from this value to dest
        void deepCopy(value &dest);

        vint &getInt();

        vbool &getBool();

        vdeci &getDeci();

        value &getRef();

        vecObject &getVec();

        funcObject &getFunc();

        lambdaObject &getLambda();

        vstr &getStr();

        managedPtr<value> operator[](const vstr &v);

        value(const vstr &v, cxtObject members);

        value(const vecObject &v, cxtObject members);

        value(cxtObject members);

        value(const funcObject &v);

        value(const lambdaObject &v);

        value(const nativeFuncPtr &v);

        value(const managedPtr<value> &v);

        value(vint v);

        value(vdeci v);

        value(vbool v);

        value();
    };

#define valueKindComparator(x, y) ((vsize)((vsize)(x) << 16) | (vsize)(y))
} // rex

#endif //REXSCRIPT_VALUE_HPP
