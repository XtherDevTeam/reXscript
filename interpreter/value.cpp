//
// Created by XIaokang00010 on 2022/12/3.
//

#include "value.hpp"

#include <memory>
#include <utility>
#include <iomanip>
#include "exceptions/rexException.hpp"

namespace rex {
    value::funcObject::funcObject() : argsName(), code() {

    }

    value::funcObject::funcObject(const managedPtr<value> &moduleCxt, const vec<vstr> &argsName, AST code) :
            moduleCxt(moduleCxt), argsName(argsName), code(std::move(code)) {

    }

    value::lambdaObject::lambdaObject() : outerCxt(), func() {

    }

    value::lambdaObject::lambdaObject(managedPtr<value> outerCxt, funcObject func) :
            outerCxt(std::move(outerCxt)), func(std::move(func)) {

    }

    bool value::isRef() {
        return kind == vKind::vRef;
    }

    void value::deepCopy(value &dest) {
        dest = *this;
        switch (dest.kind) {
            case vKind::vInt:
            case vKind::vDeci:
            case vKind::vBool:
            case vKind::vObject:
            case vKind::vNull:
            case vKind::vNativeFuncPtr:
            case vKind::vRef:
                break;
            case vKind::vStr: {
                dest.strObj = managePtr(*strObj);
                break;
            }
            case vKind::vVec: {
                dest.vecObj = managePtr(*vecObj);
                for (auto &i : *dest.vecObj) {
                    i = managePtr(*i);
                }
                break;
            }
            case vKind::vFunc: {
                dest.funcObj = managePtr(*funcObj);
                break;
            }
            case vKind::vLambda: {
                dest.lambdaObj = managePtr(*lambdaObj);
                dest.lambdaObj->outerCxt = managePtr(*dest.lambdaObj->outerCxt);
                break;
            }
        }
        for (auto &i : dest.members) {
            i.second = managePtr(*i.second);
        }
    }

    vint &value::getInt() {
        return basicValue.vInt;
    }

    vbool &value::getBool() {
        return basicValue.vBool;
    }

    vdeci &value::getDeci() {
        return basicValue.vDeci;
    }

    value &value::getRef() {
        if (refObj)
            return *refObj;
        else
            throw rexException(L"NullPointerException: refObj == nullptr");
    }

    value::vecObject &value::getVec() {
        if (vecObj)
            return *vecObj;
        else
            throw rexException(L"NullPointerException: vecObj == nullptr");
    }

    value::funcObject &value::getFunc() {
        if (funcObj)
            return *funcObj;
        else
            throw rexException(L"NullPointerException: funcObj == nullptr");
    }

    value::lambdaObject &value::getLambda() {
        if (lambdaObj)
            return *lambdaObj;
        else
            throw rexException(L"NullPointerException: lambdaObj == nullptr");
    }

    value::value(const vstr &v, value::cxtObject members) :
            kind(vKind::vStr), basicValue(), members(std::move(members)), strObj(new vstr{v}) {

    }

    value::value(const value::vecObject &v, value::cxtObject members) :
            kind(vKind::vVec), basicValue(), members(std::move(members)), vecObj(new vecObject{v}) {

    }

    value::value(const value::funcObject &v) :
            kind(vKind::vFunc), basicValue(), members(), funcObj(new funcObject{v}) {

    }

    value::value(const value::lambdaObject &v) :
            kind(vKind::vFunc), basicValue(), members(), lambdaObj(new lambdaObject{v}) {

    }

    value::value(vint v) : kind(vKind::vInt), basicValue(v), members() {

    }

    value::value(vdeci v) : kind(vKind::vDeci), basicValue(v), members() {

    }

    value::value(vbool v) : kind(vKind::vBool), basicValue(v), members() {

    }

    value::value() : kind(vKind::vNull), basicValue(), members() {

    }

    value::value(value::cxtObject members) : kind(vKind::vObject), members(std::move(members)) {

    }

    value::value(const value::nativeFuncPtr &v) :
        kind(vKind::vNativeFuncPtr), members(), nativeFuncObj(new (nativeFuncPtr){v}) {

    }

    value::value(const managedPtr<value> &v) : kind(vKind::vRef), refObj(v), members() {

    }

    managedPtr<value> value::operator[](const vstr &v) {
        if (auto it = members.find(v); it != members.end())
            return it->second;
        else
            return members[v] = managePtr(value{});
    }

    vstr &value::getStr() {
        if (strObj)
            return *strObj;
        else
            throw rexException(L"NullPointerException: strObj == nullptr");
    }

    value::operator vstr() {
        std::wstringstream ss;
        ss << L"<rexValue type=";
        switch (kind) {
            case vKind::vNull: {
                ss << L"null>";
                break;
            }
            case vKind::vInt: {
                ss << L"int val=" << basicValue.vInt << L">";
                break;
            }
            case vKind::vDeci: {
                ss << L"deci val=" << basicValue.vDeci << L">";
                break;
            }
            case vKind::vBool: {
                ss << L"bool val=" << basicValue.vBool << L">";
                break;
            }
            case vKind::vStr: {
                ss << L"str val=" << std::quoted(getStr()) << L">";
                break;
            }
            case vKind::vVec: {
                ss << L"vec val=[";
                for (vsize i = 0;i < getVec().size();i++) {
                    ss << (vstr) {*(getVec()[i])} << L",";
                }
                if (!getVec().empty())
                    ss.seekp(-1, ss.cur);
                ss << L"]>";
                break;
            }
            case vKind::vObject:
                ss << L"object val={";
                for (auto &i : members) {
                    ss << std::quoted(i.first) << ": " << (vstr){*i.second} << L',';
                }
                if (!members.empty())
                    ss.seekp(-1, ss.cur);
                ss << L"}>";
                break;
            case vKind::vFunc:
                ss << L"func>";
                break;
            case vKind::vLambda:
                ss << L"lambda>";
                break;
            case vKind::vNativeFuncPtr:
                ss << L"nativeFunc>";
                break;
            case vKind::vRef:
                ss << L"ref val=" << (vstr){getRef()} << L">";
                break;
        }
        return ss.str();
    }

    value::value(unknownPtr unk) : kind(vKind::vInt), basicValue(unk) {

    }

    value::vValue::vValue() : vInt(0) {

    }

    value::vValue::vValue(vint v) : vInt(v) {

    }

    value::vValue::vValue(vdeci v) : vDeci(v) {

    }

    value::vValue::vValue(vbool v) : vBool(v) {

    }

    value::vValue::vValue(unknownPtr unknown) : unknown(unknown) {

    }
} // rex