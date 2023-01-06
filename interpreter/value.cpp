//
// Created by XIaokang00010 on 2022/12/3.
//

#include "value.hpp"

#include <memory>
#include <utility>
#include <iomanip>
#include "exceptions/rexException.hpp"
#include <rex.hpp>

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
            case vKind::vLinkedListIter: {
                dest.linkedListIterObj = managePtr(*linkedListIterObj);
                break;
            }
            case vKind::vBytes: {
                dest.bytesObj = managePtr(*bytesObj);
                break;
            }
            case vKind::vStr: {
                dest.strObj = managePtr(*strObj);
                break;
            }
            case vKind::vVec: {
                dest.vecObj = managePtr(*vecObj);
                for (auto &i: *dest.vecObj) {
                    if (i) {
                        managedPtr<value> temp = managePtr(value{});
                        i->deepCopy(*temp);
                        i = temp;
                    }
                }
                break;
            }
            case vKind::vLinkedList: {
                dest.linkedListObj = managePtr(*linkedListObj);
                for (auto &i: *dest.linkedListObj) {
                    if (i) {
                        managedPtr<value> temp = managePtr(value{});
                        i->deepCopy(*temp);
                        i = temp;
                    }
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
        for (auto &i: dest.members) {
            if (i.second) {
                managedPtr<value> temp = managePtr(value{});
                i.second->deepCopy(*temp);
                i.second = temp;
            }
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
            kind(vKind::vLambda), basicValue(), members(), lambdaObj(new lambdaObject{v}) {

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
            kind(vKind::vNativeFuncPtr), basicValue(), members(), nativeFuncObj(new(nativeFuncPtr){v}) {

    }

    value::value(const managedPtr<value> &v) : kind(vKind::vRef), basicValue(), refObj(v), members() {

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
        switch (kind) {
            case vKind::vNull: {
                ss << L"null";
                break;
            }
            case vKind::vInt: {
                ss << basicValue.vInt;
                break;
            }
            case vKind::vDeci: {
                ss << basicValue.vDeci;
                break;
            }
            case vKind::vBool: {
                ss << basicValue.vBool;
                break;
            }
            case vKind::vStr: {
                ss << std::quoted(getStr());
                break;
            }
            case vKind::vVec: {
                ss << L"[";
                for (auto &i: getVec()) {
                    ss << (vstr) {*i} << L",";
                }
                if (!getVec().empty())
                    ss.seekp(-1, ss.cur);
                ss << L"]";
                break;
            }
            case vKind::vLinkedList: {
                ss << L"[";
                for (auto &i: getLinkedList()) {
                    ss << (vstr) {*i} << L",";
                }
                if (!getLinkedList().empty())
                    ss.seekp(-1, ss.cur);
                ss << L"]";
                break;
            }
            case vKind::vObject:
                ss << L"{";
                for (auto &i: members) {
                    ss << std::quoted(i.first) << ": " << (vstr) {*i.second} << L',';
                }
                if (!members.empty())
                    ss.seekp(-1, ss.cur);
                ss << L"}";
                break;
            case vKind::vFunc:
                ss << L"<func addr=" << &funcObj << ">";
                break;
            case vKind::vLambda:
                ss << L"<lambda addr=" << &lambdaObj << ">";
                break;
            case vKind::vNativeFuncPtr:
                ss << L"<nativeFn addr=" << &nativeFuncObj << ">";
                break;
            case vKind::vLinkedListIter:
                ss << L"<linkedListIter addr=" << &linkedListIterObj << ">";
                break;
            case vKind::vRef:
                ss << (vstr) {getRef()};
                break;
            case vKind::vBytes:
                ss << L"b" << std::quoted(string2wstring(getPrintableBytes(getBytes())));
                break;
        }
        return ss.str();
    }

    value::value(unknownPtr unk) : kind(vKind::vInt), basicValue(unk) {

    }

    value::value(const vbytes &v, value::cxtObject members) : kind(vKind::vBytes), bytesObj(managePtr(v)),
                                                              members(std::move(members)) {

    }

    vbytes &value::getBytes() {
        if (bytesObj)
            return *bytesObj;
        else
            throw rexException(L"NullPointerException: bytesObj == nullptr");
    }

    value::linkedListObject &value::getLinkedList() {
        if (linkedListObj)
            return *linkedListObj;
        else
            throw rexException(L"NullPointerException: linkedListObj == nullptr");
    }

    value::value(const value::linkedListObject &v, value::cxtObject members) : kind(vKind::vLinkedList),
                                                                               linkedListObj(managePtr(v)),
                                                                               members(std::move(members)) {

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

    value::value(const linkedListObject::iterator &v) : kind(vKind::vLinkedListIter), linkedListIterObj(managePtr(v)) {}

    vstr value::getKind() {
        switch (kind) {
            case vKind::vNull:
                return L"null";
            case vKind::vInt:
                return L"int";
            case vKind::vDeci:
                return L"deci";
            case vKind::vBool:
                return L"bool";
            case vKind::vStr:
                return L"str";
            case vKind::vBytes:
                return L"bytes";
            case vKind::vVec:
                return L"vec";
            case vKind::vObject:
                return L"object";
            case vKind::vFunc:
                return L"func";
            case vKind::vLambda:
                return L"lambda";
            case vKind::vNativeFuncPtr:
                return L"nativeFuncPtr";
            case vKind::vRef:
                return getRef().getKind();
            case vKind::vLinkedList:
                return L"linkedList";
            case vKind::vLinkedListIter:
                return L"linkedListIter";
        }
    }

    value::~value() {
        if (kind == vKind::vObject) {
            if (auto it = members.find(L"finalize"); it != members.end()) {
                if (it->second->basicValue.vInt != 0x114514ccf) {
                    auto in = managePtr(interpreter{rexEnvironmentInstance, {}});
                    it->second->basicValue.vInt = 0x114514ccf;
                    in->invokeFunc(it->second, {}, managePtr(*this));
                }
            }
        }
    }
} // rex