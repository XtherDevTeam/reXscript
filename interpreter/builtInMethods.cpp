//
// Created by XIaokang00010 on 2022/12/3.
//

#include <iostream>
#include <iomanip>
#include "builtInMethods.hpp"
#include "interpreter/interpreter.hpp"
#include "rex.hpp"
#include "exceptions/signalException.hpp"
#include "share/share.hpp"

namespace rex {
    value::cxtObject stringMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"substr"] = managePtr(value{(value::nativeFuncPtr) substr});
        result[L"startsWith"] = managePtr(value{(value::nativeFuncPtr) startsWith});
        result[L"endsWith"] = managePtr(value{(value::nativeFuncPtr) endsWith});
        result[L"charAt"] = managePtr(value{(value::nativeFuncPtr) charAt});
        result[L"fromChar"] = managePtr(value{(value::nativeFuncPtr) fromChar});
        result[L"length"] = managePtr(value{(value::nativeFuncPtr) length});
        result[L"rexEqual"] = managePtr(value{(value::nativeFuncPtr) rexEqual});
        result[L"rexNotEqual"] = managePtr(value{(value::nativeFuncPtr) rexNotEqual});
        result[L"rexAdd"] = managePtr(value{(value::nativeFuncPtr) rexAdd});
        result[L"rexAddAssign"] = managePtr(value{(value::nativeFuncPtr) rexAddAssign});
        result[L"join"] = managePtr(value{(value::nativeFuncPtr) join});
        return result;
    }

    nativeFn(stringMethods::substr, interpreter, args, passThisPtr) {
        switch (args.size()) {
            case 1: {
                if (args[0].isRef())
                    args[0] = args[0].getRef();
                return {passThisPtr->getStr().substr(args[0].getInt()), getMethodsCxt()};
            }
            case 2: {
                if (args[0].isRef())
                    args[0] = args[0].getRef();
                if (args[1].isRef())
                    args[1] = args[1].getRef();
                return {passThisPtr->getStr().substr(args[0].getInt(), args[1].getInt()), getMethodsCxt()};
            }
            default: {
                throw rex::signalException(
                        rex::interpreter::makeErr(L"argumentsError", L"substr() expected one or two arguments"));
            }
        }
    }

    nativeFn(stringMethods::startsWith, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();
        return {passThisPtr->getStr().starts_with(args[0].getStr())};
    }

    nativeFn(stringMethods::endsWith, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();
        return {passThisPtr->getStr().ends_with(args[0].getStr())};
    }

    nativeFn(stringMethods::charAt, interpreter, args, passThisPtr) {
        return {(vint) passThisPtr->getStr()[args[0].getInt()]};
    }

    nativeFn(stringMethods::fromChar, interpreter, args, passThisPtr) {
        passThisPtr->getStr() = (vchar) args[0].getInt();
        return passThisPtr;
    }

    nativeFn(stringMethods::length, interpreter, args, passThisPtr) {
        return {(vint) passThisPtr->getStr().length()};
    }

    nativeFn(stringMethods::rexEqual, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return {passThisPtr->getStr() == args[0].getStr()};
    }

    nativeFn(stringMethods::rexNotEqual, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return {passThisPtr->getStr() != args[0].getStr()};
    }

    nativeFn(stringMethods::rexAdd, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return {passThisPtr->getStr() + args[0].getStr(), getMethodsCxt()};
    }

    nativeFn(stringMethods::rexAddAssign, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();

        if (passThisPtr.unique())
            passThisPtr->getStr() += args[0].getStr();
        else
            passThisPtr->strObj = managePtr(
                    passThisPtr->getStr() + args[0].getStr()); // not unique, use copy construct instead

        return passThisPtr;
    }

    nativeFn(stringMethods::join, interpreter, args, passThisPtr) {
        for (auto &i: args)
            if (i.isRef())
                i = i.getRef();

        value result = {L"", getMethodsCxt()};
        if (!args.empty())
            result.getStr() += args[0].getStr();

        for (vsize i = 1; i < args.size(); i++) {
            result.getStr() += passThisPtr->getStr();
            result.getStr() += args[i].getStr();
        }

        return result;
    }

    nativeFn(stringMethods::encode, interpreter, args, passThisPtr) {
        // TODO: Add charset support
    }

    value::cxtObject vecMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"append"] = managePtr(value{(value::nativeFuncPtr) append});
        result[L"rexEqual"] = managePtr(value{(value::nativeFuncPtr) rexEqual});
        result[L"rexNotEqual"] = managePtr(value{(value::nativeFuncPtr) rexNotEqual});
        return result;
    }

    nativeFn(vecMethods::append, interpreter, args, passThisPtr) {
        for (auto &i: args)
            passThisPtr->getVec().push_back(managePtr(i));
        return passThisPtr;
    }

    nativeFn(vecMethods::rexEqual, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();
        if (args[0].kind != value::vKind::vVec or passThisPtr->getVec().size() != args[0].getVec().size())
            return {false};
        for (auto it = args[0].getVec().begin(), it1 = passThisPtr->getVec().begin();
             it != args[0].getVec().end(); it++, it1++) {
            if (!in->opEqual(*(*it), *(*it1)).getBool())
                return {false};
        }
        return {true};
    }

    nativeFn(vecMethods::rexNotEqual, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();
        if (args[0].kind != value::vKind::vVec or passThisPtr->getVec().size() != args[0].getVec().size())
            return {true};
        for (auto it = args[0].getVec().begin(), it1 = passThisPtr->getVec().begin();
             it != args[0].getVec().end(); it++, it1++) {
            if (in->opNotEqual(*(*it), *(*it1)).getBool())
                return {true};
        }
        return {false};
    }

    nativeFn(globalMethods::input, interpreter, args, passThisPtr) {
        std::string s;
        std::getline(std::cin, s);
        return {string2wstring(s), rex::stringMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::print, interpreter, args, passThisPtr) {
        for (auto &i: args) {
            switch (i.kind) {
                case value::vKind::vBool:
                    std::cout << i.getBool();
                    break;
                case value::vKind::vInt:
                    std::cout << i.getInt();
                    break;
                case value::vKind::vDeci:
                    std::cout << i.getDeci();
                    break;
                case value::vKind::vStr:
                    std::cout << wstring2string(i.getStr());
                    break;
                default:
                    std::cout << wstring2string(i);
                    break;
            }
        }
        return {};
    }

    value::cxtObject globalMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"input"] = managePtr(value{(value::nativeFuncPtr) input});
        result[L"print"] = managePtr(value{(value::nativeFuncPtr) print});
        result[L"import"] = managePtr(value{(value::nativeFuncPtr) rexImport});
        result[L"nativeImport"] = managePtr(value{(value::nativeFuncPtr) rexNativeImport});
        result[L"format"] = managePtr(value{(value::nativeFuncPtr) format});
        result[L"threading"] = managePtr(threadingMethods::getThreadingModule());
        result[L"importPrefixPath"] = managePtr(value{value::vecObject{
                managePtr(value{L"", stringMethods::getMethodsCxt()})
        }, rex::vecMethods::getMethodsCxt()});
        return result;
    }

    nativeFn(globalMethods::rexImport, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return rex::importExternModules(in->env, args[0].getStr());
    }

    nativeFn(globalMethods::rexNativeImport, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return rex::importNativeModules(in->env, args[0].getStr());
    }

    nativeFn(globalMethods::format, interpreter, args, passThisPtr) {
        for (auto &i: args)
            if (i.isRef())
                i = i.getRef();

        formatterTagInfo tag;
        value result{L"", rex::stringMethods::getMethodsCxt()};

        vsize cur = 1;

        for (vchar *ch = &args[0].getStr().front(); *ch; ch++) {
            if (*ch == L'$' && *(ch + 1) == L'{') {
                ch += 2;
                if (ch < &args[0].getStr().back()) {
                    tag = {};
                    ch = tag.parse(ch);

                    if (ch) {
                        switch (tag.kind) {
                            case formatterTagInfo::tagKind::vStr:
                                result.getStr() += args[cur].getStr();
                                cur++;
                                break;
                            case formatterTagInfo::tagKind::vInt: {
                                std::wstringstream ss;
                                switch (tag.base) {
                                    case formatterTagInfo::tagBase::hex:
                                        ss << std::hex << args[cur].getInt();
                                        break;
                                    case formatterTagInfo::tagBase::dec:
                                        ss << std::dec << args[cur].getInt();
                                        break;
                                    case formatterTagInfo::tagBase::oct:
                                        ss << std::oct << args[cur].getInt();
                                        break;
                                }
                                result.getStr() += ss.str();
                                cur++;
                                break;
                            }
                            case formatterTagInfo::tagKind::vDeci: {
                                std::wstringstream ss;
                                ss << std::fixed << std::setprecision((int) tag.precision) << args[cur].getDeci();
                                result.getStr() += ss.str();
                                cur++;
                                break;
                            }
                            case formatterTagInfo::tagKind::vBool: {
                                result.getStr() += args[cur].getBool() ? L"true" : L"false";
                                cur++;
                                break;
                            }
                            case formatterTagInfo::tagKind::dollar: {
                                result.getStr() += L'$';
                                break;
                            }
                        }
                        if (*ch != L'}')
                            throw signalException(
                                    interpreter::makeErr(L"formatError", L"expected `}` after the formatter tag"));
                    } else {
                        throw signalException(interpreter::makeErr(L"formatError", L"invalid formatter tag"));
                    }
                }
            } else {
                result.getStr() += *ch;
            }
        }

        return result;
    }

    formatterTagInfo::formatterTagInfo() :
            kind(tagKind::vStr), base(tagBase::dec), precision(6) {

    }

    vchar *formatterTagInfo::parse(vchar *str) {
        vstr a;
        vstr b;

        while (isalpha(*str)) if (*str == L'\0') { return nullptr; } else a += *(str++);
        if (a == L"str") {
            kind = tagKind::vStr;
        } else if (a == L"int") {
            kind = tagKind::vInt;
        } else if (a == L"deci") {
            kind = tagKind::vDeci;
        } else if (a == L"bool") {
            kind = tagKind::vBool;
        } else if (a == L"dol") {
            kind = tagKind::dollar;
        } else {
            return nullptr;
        }
        if (kind == tagKind::vInt || kind == tagKind::vDeci) {
            a = L"";
            str++;
            while (isalpha(*str)) if (*str == L'\0') { return nullptr; } else a += *(str++);

            str++;
            while (isalpha(*str) or isdigit(*str)) if (*str == L'\0') { return nullptr; } else b += *(str++);

            if (a == L"bs") {
                if (b == L"hex")
                    base = tagBase::hex;
                else if (b == L"dec")
                    base = tagBase::dec;
                else if (b == L"oct")
                    base = tagBase::oct;
                else
                    return nullptr;
            } else if (a == L"pre") {
                try {
                    precision = std::stol(b);
                } catch (...) {
                    return nullptr;
                }
            } else {
                return nullptr;
            }
        }
        return str;
    }

    nativeFn(threadingMethods::start, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        vec<value> thArgs;
        for (vint i = 1; i < args.size(); i++) {
            value temp;
            args[i].deepCopy(temp);
            thArgs.push_back(temp);
        }
        return {spawnThread(in->env, in->moduleCxt, args[0].isRef() ? args[0].refObj : managePtr(args[0]), thArgs)};
    }

    nativeFn(threadingMethods::wait, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        return waitForThread(in->env, args[0].isRef() ? args[0].getRef().getInt() : args[0].getInt());
    }

    value::cxtObject threadingMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"start"] = managePtr(value{(value::nativeFuncPtr) start});
        result[L"wait"] = managePtr(value{(value::nativeFuncPtr) wait});

        return result;
    }

    value threadingMethods::getThreadingModule() {
        return {getMethodsCxt()};
    }

    nativeFn(bytesMethods::length, interpreter, args, passThisPtr) {
        return {(vint)passThisPtr->bytesObj->length()};
    }

    nativeFn(bytesMethods::rexEqual, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return {passThisPtr->getBytes() == args[0].getBytes()};
    }

    nativeFn(bytesMethods::rexNotEqual, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return {passThisPtr->getBytes() != args[0].getBytes()};
    }

    nativeFn(bytesMethods::concat, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();

        passThisPtr->getBytes() += args[0].getBytes();
        return {};
    }

    nativeFn(bytesMethods::decode, interpreter, args, passThisPtr) {
        // TODO: Add charset support
    }

    value::cxtObject bytesMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"length"] = managePtr(value{(value::nativeFuncPtr) length});
        result[L"rexEqual"] = managePtr(value{(value::nativeFuncPtr) rexEqual});
        result[L"rexNotEqual"] = managePtr(value{(value::nativeFuncPtr) rexNotEqual});
        result[L"concat"] = managePtr(value{(value::nativeFuncPtr) concat});

        return result;
    }
}