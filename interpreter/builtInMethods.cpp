#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-unnecessary-value-param"
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
#include "exceptions/signalBreak.hpp"
#include "exceptions/signalContinue.hpp"

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
        result[L"encode"] = managePtr(value{(value::nativeFuncPtr) encode});
        result[L"trim"] = managePtr(value{(value::nativeFuncPtr) trim});
        result[L"split"] = managePtr(value{(value::nativeFuncPtr) split});
        result[L"contains"] = managePtr(value{(value::nativeFuncPtr) contains});
        return result;
    }

    nativeFn(stringMethods::contains, interpreter, args, passThisPtr) {
        if (passThisPtr->getStr().find(eleGetRef(args[0]).getStr()) != vstr::npos) {
            return true;
        } else {
            return false;
        }
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

    nativeFn(stringMethods::trim, interpreter, args, passThisPtr) {
        vstr s = passThisPtr->getStr();
        s.erase(0, s.find_first_not_of(L' '));
        s.erase(s.find_last_not_of(L' ') + 1);
        return {s, getMethodsCxt()};
    }

    nativeFn(stringMethods::split, interpreter, args, passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();
        value result{value::vecObject{}, rex::vecMethods::getMethodsCxt()};
        rex::split(passThisPtr->getStr(), args[0].getStr(), [&](const vstr &r, vsize _) {
            result.getVec().push_back(managePtr(value{r, getMethodsCxt()}));
        });
        return result;
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

        passThisPtr->getStr() += args[0].getStr();

        return passThisPtr;
    }

    nativeFn(stringMethods::join, interpreter, args, passThisPtr) {
        for (auto &i: args)
            if (i.isRef())
                getSelfRef(i);

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
        auto in = static_cast<rex::interpreter *>(interpreter);
        vstr &charsetName = eleGetRef(args[0]).getStr();
        auto encoder = in->env->globalCxt->members[L"charsets"]->members[charsetName]->members[L"encoder"];
        return in->invokeFunc(encoder, {passThisPtr}, {});
    }

    value::cxtObject vecMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"contains"] = managePtr(value{(value::nativeFuncPtr) contains});
        result[L"append"] = managePtr(value{(value::nativeFuncPtr) append});
        result[L"pop"] = managePtr(value{(value::nativeFuncPtr) pop});
        result[L"remove"] = managePtr(value{(value::nativeFuncPtr) remove});
        result[L"removeAll"] = managePtr(value{(value::nativeFuncPtr) removeAll});
        result[L"length"] = managePtr(value{(value::nativeFuncPtr) length});
        result[L"rexEqual"] = managePtr(value{(value::nativeFuncPtr) rexEqual});
        result[L"rexNotEqual"] = managePtr(value{(value::nativeFuncPtr) rexNotEqual});
        result[L"rexIter"] = managePtr(value{(value::nativeFuncPtr) rexIter});
        return result;
    }

    nativeFn(vecMethods::length, interpreter, args, passThisPtr) {
        return {(vint) passThisPtr->getVec().size()};
    }

    nativeFn(vecMethods::contains, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        value lhs = eleGetRef(args[0]);
        for (auto it = passThisPtr->getVec().begin(); it != passThisPtr->getVec().end();) {
            if (in->opEqual(lhs, **it).getBool()) {
                return true;
            }
        }
        return false;
    }

    nativeFn(vecMethods::append, interpreter, args, passThisPtr) {
        for (auto &i: args)
            passThisPtr->getVec().push_back(managePtr(eleGetRef(i)));
        return passThisPtr;
    }

    nativeFn(vecMethods::pop, interpreter, args, passThisPtr) {
        passThisPtr->getVec().pop_back();
        return passThisPtr;
    }

    nativeFn(vecMethods::remove, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        value lhs = eleGetRef(args[0]);
        for (auto it = passThisPtr->getVec().begin(); it != passThisPtr->getVec().end();) {
            if (in->opEqual(lhs, **it).getBool()) {
                passThisPtr->getVec().erase(it);
                break;
            } else {
                it++;
            }
        }

        return passThisPtr;
    }

    nativeFn(vecMethods::removeAll, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        value lhs = eleGetRef(args[0]);
        for (auto it = passThisPtr->getVec().begin(); it != passThisPtr->getVec().end();) {
            if (in->opEqual(lhs, **it).getBool())
                it = passThisPtr->getVec().erase(it);
            else
                it++;
        }

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

    nativeFn(vecMethods::rexIter, interpreter, args, passThisPtr) {
        return {iterator::getMethodsCxt(passThisPtr->getVec())};
    }

    nativeFn(globalMethods::rexDeci, interpreter, args, passThisPtr) {
        value &ref = eleGetRef(args[0]);
        switch (ref.kind) {
            case value::vKind::vInt:
                return (vdeci) ref.getInt();
            case value::vKind::vDeci:
                return ref;
            case value::vKind::vBool:
                return (vdeci) ref.getBool();
            case value::vKind::vStr:
                return (vdeci) std::stod(ref.getStr());
            default:
                throw signalException(interpreter::makeErr(L"typeError", L"cannot covert to decimal"));
        }
    }

    nativeFn(globalMethods::rexInt, interpreter, args, passThisPtr) {
        value &ref = eleGetRef(args[0]);
        switch (ref.kind) {
            case value::vKind::vInt:
                return ref;
            case value::vKind::vDeci:
                return (vint) ref.getDeci();
            case value::vKind::vBool:
                return (vint) ref.getBool();
            case value::vKind::vStr:
                return (vint) std::stol(ref.getStr(), nullptr,
                                        args.size() > 1 ? (int) eleGetRef(args[1]).getInt() : 10);
            default:
                throw signalException(interpreter::makeErr(L"typeError", L"cannot covert to integer"));
        }
    }

    nativeFn(globalMethods::input, interpreter, args, passThisPtr) {
        std::string s;
        std::getline(std::cin, s);
        return {string2wstring(s), rex::stringMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::stringify, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        auto &v = eleGetRef(args[0]);
        value result = {L"", stringMethods::getMethodsCxt()};
        if (auto it = v.members.find(L"rexStr"); v.kind == value::vKind::vObject and it != v.members.end()) {
            result.getStr() += in->invokeFunc(it->second, {}, managePtr(v)).getStr();
        } else {
            result.getStr() += v;
        }
        return result;
    }

    nativeFn(globalMethods::print, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        for (auto &item: args) {
            auto &i = eleGetRef(item);
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
                    if (auto it = i.members.find(L"rexStr"); it != i.members.end()) {
                        std::cout << wstring2string(
                                in->invokeFunc(it->second, {}, eleRefObj(item)).getStr());
                    } else {
                        std::cout << wstring2string(i);
                    }
                    break;
            }
        }
        return {};
    }

    nativeFn(globalMethods::hash, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        value &element = eleGetRef(args[0]);
        switch (element.kind) {
            case value::vKind::vInt:
                return (unknownPtr) std::hash<vint>()(element.getInt());
            case value::vKind::vDeci:
                return (unknownPtr) std::hash<vdeci>()(element.getDeci());
            case value::vKind::vBool:
                return (unknownPtr) std::hash<vbool>()(element.getBool());
            case value::vKind::vStr:
                return (unknownPtr) std::hash<vstr>()(element.getStr());
            case value::vKind::vBytes:
                return (unknownPtr) std::hash<vbytes>()(element.getBytes());
            default:
                if (auto it = element.members.find(L"rexHash"); it != element.members.end()) {
                    return in->invokeFunc(it->second, {}, args[0].isRef() ? args[0].refObj : managePtr(args[0]));
                } else {
                    throw signalException(interpreter::makeErr(L"hashError", L"`rexHash` not implemented"));
                }
        }
    }

    nativeFn(globalMethods::type, interpreter, args, passThisPtr) {
        return {args[0].getKind(), stringMethods::getMethodsCxt()};
    }

    value::cxtObject globalMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"int"] = managePtr(value{(value::nativeFuncPtr) rexInt});
        result[L"deci"] = managePtr(value{(value::nativeFuncPtr) rexDeci});
        result[L"input"] = managePtr(value{(value::nativeFuncPtr) input});
        result[L"print"] = managePtr(value{(value::nativeFuncPtr) print});
        result[L"requireMod"] = managePtr(value{(value::nativeFuncPtr) rexRequireMod});
        result[L"requireNativeMod"] = managePtr(value{(value::nativeFuncPtr) rexRequireNativeMod});
        result[L"requirePackage"] = managePtr(value{(value::nativeFuncPtr) rexRequirePackage});
        result[L"require"] = managePtr(value{(value::nativeFuncPtr) rexRequire});
        result[L"format"] = managePtr(value{(value::nativeFuncPtr) format});
        result[L"hash"] = managePtr(value{(value::nativeFuncPtr) hash});
        result[L"linkedList"] = managePtr(value{(value::nativeFuncPtr) linkedList});
        result[L"hashMap"] = managePtr(value{(value::nativeFuncPtr) hashMap});
        result[L"stringify"] = managePtr(value{(value::nativeFuncPtr) stringify});
        result[L"iter"] = managePtr(value{iterMethods::getMethodsCxt()});
        result[L"object"] = managePtr(value{objectMethods::getMethodsCxt()});
        result[L"type"] = managePtr(value{(value::nativeFuncPtr) type});

        result[L"mutex"] = managePtr(value{(value::nativeFuncPtr) mutex});

        result[L"threading"] = managePtr(threadingMethods::getThreadingModule());
        result[L"importPrefixPath"] = managePtr(value{value::vecObject{
                managePtr(value{L"./modules", stringMethods::getMethodsCxt()}),
                managePtr(value{string2wstring(getRexExecPath()) + L"/modules", stringMethods::getMethodsCxt()}),
                managePtr(value{L"", stringMethods::getMethodsCxt()}),
        }, rex::vecMethods::getMethodsCxt()});

        result[L"rexPlatform"] = managePtr(value{getOSName(), stringMethods::getMethodsCxt()});
        result[L"rexArch"] = managePtr(value{getCPUArch(), stringMethods::getMethodsCxt()});
        result[L"rexDylibSuffix"] = managePtr(value{getDylibSuffix(), stringMethods::getMethodsCxt()});

        result[L"charsets"] = managePtr(value{value::cxtObject{}});

        // initialize default charsets
        result[L"charsets"]->members[L"ansi"] = managePtr(value{value::cxtObject{}});
        result[L"charsets"]->members[L"utf-8"] = managePtr(value{value::cxtObject{}});

        result[L"charsets"]->members[L"ansi"]->members[L"encoder"] =
                managePtr(value{(value::nativeFuncPtr) charsetsMethods::ansiEncoder});
        result[L"charsets"]->members[L"ansi"]->members[L"decoder"] =
                managePtr(value{(value::nativeFuncPtr) charsetsMethods::ansiDecoder});

        result[L"charsets"]->members[L"utf-8"]->members[L"encoder"] =
                managePtr(value{(value::nativeFuncPtr) charsetsMethods::utf8Encoder});
        result[L"charsets"]->members[L"utf-8"]->members[L"decoder"] =
                managePtr(value{(value::nativeFuncPtr) charsetsMethods::utf8Decoder});


        return result;
    }

    nativeFn(globalMethods::charsetsMethods::ansiEncoder, interpreter, args, passThisPtr) {
        vstr &str = eleGetRef(args[0]).getStr();
        vbytes output{};

        for (auto &i: str) {
            output.push_back(i < 256 ? (vbyte) i : '?');
        }

        return {output, bytesMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::charsetsMethods::ansiDecoder, interpreter, args, passThisPtr) {
        vbytes &str = eleGetRef(args[0]).getBytes();
        vstr output{};

        for (auto &i: str) {
            output += i;
        }

        return {output, stringMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::charsetsMethods::utf8Encoder, interpreter, args, passThisPtr) {
        vstr &str = eleGetRef(args[0]).getStr();
        vbytes output{wstring2string(str)};

        return {output, bytesMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::charsetsMethods::utf8Decoder, interpreter, args, passThisPtr) {
        vbytes &str = eleGetRef(args[0]).getBytes();
        vstr output{string2wstring(str)};

        return {output, stringMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::rexRequireMod, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        value::cxtObject defCxt;
        if (auto it = in->moduleCxt->members.find(L"rexPkgRoot"); it != in->moduleCxt->members.end()) {
            defCxt.insert(*it);
        }

        return rex::importExternModule(in, args[0].getStr());
    }

    nativeFn(globalMethods::mutex, interpreter, args, passThisPtr) {
        return mutexMethods::getMethodsCxt(new std::mutex());
    }

    nativeFn(globalMethods::rexRequireNativeMod, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        value::cxtObject defCxt;
        if (auto it = in->moduleCxt->members.find(L"rexPkgRoot"); it != in->moduleCxt->members.end()) {
            defCxt.insert(*it);
        }

        return rex::importNativeModule(in, args[0].getStr());
    }

    nativeFn(globalMethods::rexRequirePackage, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return rex::importExternPackage(in, args[0].getStr());
    }

    nativeFn(globalMethods::rexRequire, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return importEx(in, args[0].getStr());
    }

    nativeFn(globalMethods::format, interpreter, args, passThisPtr) {
        for (auto &i: args)
            if (i.isRef())
                getSelfRef(i);

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

    nativeFn(threadingMethods::sleep, interpreter, args, passThisPtr) {
        std::this_thread::sleep_for(
                std::chrono::milliseconds(args[0].isRef() ? args[0].getRef().getInt() : args[0].getInt()));
        return {};
    }

    value::cxtObject threadingMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"start"] = managePtr(value{(value::nativeFuncPtr) start});
        result[L"wait"] = managePtr(value{(value::nativeFuncPtr) wait});
        result[L"sleep"] = managePtr(value{(value::nativeFuncPtr) sleep});

        return result;
    }

    value threadingMethods::getThreadingModule() {
        return {getMethodsCxt()};
    }

    nativeFn(bytesMethods::contains, interpreter, args, passThisPtr) {
        if (passThisPtr->getBytes().find(eleGetRef(args[0]).getBytes()) != vbytes::npos) {
            return true;
        } else {
            return false;
        }
    }

    nativeFn(bytesMethods::slice, interpreter, args, passThisPtr) {
        return {passThisPtr->getBytes().substr(eleGetRef(args[0]).getInt(), eleGetRef(args[1]).getInt()),
                bytesMethods::getMethodsCxt()};
    }

    nativeFn(bytesMethods::length, interpreter, args, passThisPtr) {
        return {(vint) passThisPtr->bytesObj->length()};
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
        auto in = static_cast<rex::interpreter *>(interpreter);
        vstr &charsetName = eleGetRef(args[0]).getStr();
        auto decoder = in->env->globalCxt->members[L"charsets"]->members[charsetName]->members[L"decoder"];
        return in->invokeFunc(decoder, {passThisPtr}, {});
    }

    value::cxtObject bytesMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"length"] = managePtr(value{(value::nativeFuncPtr) length});
        result[L"rexEqual"] = managePtr(value{(value::nativeFuncPtr) rexEqual});
        result[L"rexNotEqual"] = managePtr(value{(value::nativeFuncPtr) rexNotEqual});
        result[L"concat"] = managePtr(value{(value::nativeFuncPtr) concat});
        result[L"decode"] = managePtr(value{(value::nativeFuncPtr) decode});
        result[L"slice"] = managePtr(value{(value::nativeFuncPtr) slice});
        result[L"contains"] = managePtr(value{(value::nativeFuncPtr) contains});

        return result;
    }

    value::cxtObject vecMethods::iterator::getMethodsCxt(const value::vecObject &container) {
        value::cxtObject result;
        result[L"container"] = managePtr(value{container, vecMethods::getMethodsCxt()});
        result[L"cur"] = managePtr(value{(vint) 0});
        result[L"next"] = managePtr(value{(value::nativeFuncPtr) next});
        return result;
    }

    nativeFn(vecMethods::iterator::next, interpreter, args, passThisPtr) {
        auto container =
                passThisPtr->members[L"container"]->isRef() ? passThisPtr->members[L"container"]->getRef().getVec()
                                                            : passThisPtr->members[L"container"]->getVec();
        auto &index = passThisPtr->members[L"cur"]->getInt();
        if (index >= container.size())
            return interpreter::makeIt({}, true);
        auto res = container[index]->isRef() ? container[index]->refObj : container[index];
        index++;
        return interpreter::makeIt(res, false);
    }

    value::cxtObject linkedListMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"contains"] = managePtr(value{(value::nativeFuncPtr) contains});
        result[L"append"] = managePtr(value{(value::nativeFuncPtr) append});
        result[L"pop"] = managePtr(value{(value::nativeFuncPtr) pop});
        result[L"remove"] = managePtr(value{(value::nativeFuncPtr) remove});
        result[L"removeAll"] = managePtr(value{(value::nativeFuncPtr) removeAll});
        result[L"rexIter"] = managePtr(value{(value::nativeFuncPtr) rexIter});
        return result;
    }

    nativeFn(linkedListMethods::contains, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        value lhs = eleGetRef(args[0]);
        for (auto it = passThisPtr->getLinkedList().begin(); it != passThisPtr->getLinkedList().end();) {
            if (in->opEqual(lhs, **it).getBool()) {
                return true;
            }
        }
        return false;
    }

    nativeFn(linkedListMethods::append, interpreter, args, passThisPtr) {
        passThisPtr->getLinkedList().push_back(args[0].isRef() ? args[0].refObj : managePtr(args[0]));
        return {};
    }

    nativeFn(linkedListMethods::pop, interpreter, args, passThisPtr) {
        passThisPtr->getLinkedList().pop_back();
        return {};
    }

    nativeFn(linkedListMethods::remove, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        value lhs = eleGetRef(args[0]);
        for (auto it = passThisPtr->getLinkedList().begin(); it != passThisPtr->getLinkedList().end();) {
            if (in->opEqual(lhs, **it).getBool()) {
                passThisPtr->getLinkedList().erase(it);
                break;
            } else {
                it++;
            }
        }

        return passThisPtr;
    }

    nativeFn(linkedListMethods::removeAll, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        value lhs = eleGetRef(args[0]);
        for (auto it = passThisPtr->getLinkedList().begin(); it != passThisPtr->getLinkedList().end();) {
            if (in->opEqual(lhs, **it).getBool())
                it = passThisPtr->getLinkedList().erase(it);
            else
                it++;
        }

        return passThisPtr;
    }

    nativeFn(linkedListMethods::rexIter, interpreter, args, passThisPtr) {
        return {iterator::getMethodsCxt(passThisPtr->getLinkedList())};
    }

    value::cxtObject linkedListMethods::iterator::getMethodsCxt(value::linkedListObject &container) {
        value::cxtObject result;
        result[L"container"] = managePtr(value{container, linkedListMethods::getMethodsCxt()});
        result[L"cur"] = managePtr(value{result[L"container"]->getLinkedList().begin()});
        result[L"next"] = managePtr(value{(value::nativeFuncPtr) next});
        return result;
    }

    nativeFn(linkedListMethods::iterator::next, interpreter, args, passThisPtr) {
        auto &container = passThisPtr->members[L"container"]->getLinkedList();
        auto &iter = passThisPtr->members[L"cur"]->linkedListIterObj;
        auto &element = **iter;
        if (*iter == container.end()) {
            return interpreter::makeIt({}, true);
        }
        (*iter)++;

        return interpreter::makeIt(element->isRef() ? element->refObj : element, false);
    }

    nativeFn(globalMethods::linkedList, interpreter, args, passThisPtr) {
        value result{value::linkedListObject{}, linkedListMethods::getMethodsCxt()};
        if (args.size() == 1) {
            for (auto &i: args[0].getVec()) {
                result.getLinkedList().push_back(i);
            }
        } else {
            for (auto &i: args) {
                result.getLinkedList().push_back(i.isRef() ? i.refObj : managePtr(i));
            }
        }
        return result;
    }

    value::cxtObject hashMapMethods::getMethodsCxt(vint defaultHashTSize) {
        value::cxtObject result{};
        // Methods
        result[L"insert"] = managePtr(value{value::nativeFuncPtr{insert}});
        result[L"realloc"] = managePtr(value{value::nativeFuncPtr{realloc}});
        result[L"remove"] = managePtr(value{value::nativeFuncPtr{remove}});
        result[L"rexIndex"] = managePtr(value{value::nativeFuncPtr{rexIndex}});
        result[L"keys"] = managePtr(value{value::nativeFuncPtr{keys}});
        result[L"rexIter"] = managePtr(value{value::nativeFuncPtr{rexIter}});
        result[L"rexClone"] = managePtr(value{value::nativeFuncPtr{rexClone}});
        result[L"rexStr"] = managePtr(value{value::nativeFuncPtr{rexStr}});
        result[L"toObject"] = managePtr(value{value::nativeFuncPtr{toObject}});
        // Members
        result[L"kvPairs"] = managePtr(value{value::linkedListObject{}, linkedListMethods::getMethodsCxt()});
        result[L"hashT"] = managePtr(value{value::vecObject{}, vecMethods::getMethodsCxt()});
        result[L"hashT"]->getVec().resize(defaultHashTSize);
        return result;
    }

    nativeFn(hashMapMethods::toObject, interpreter, args, passThisPtr) {
        auto &container = passThisPtr->members[L"kvPairs"]->getLinkedList();
        value v{value::cxtObject{}};
        for (auto &i: container) {
            value &left = eleGetRef(*i->getVec()[1]);
            if (left.kind == value::vKind::vStr) {
                v.members[left.getStr()] = eleRefObj(*i->getVec()[2]);
            } else {
                throw signalException(interpreter::makeErr(L"hashMapError", L"the left hand side is not string"));
            }
        }
        return v;
    }

    nativeFn(hashMapMethods::fromObject, interpreter, args, passThisPtr) {
        value &v = eleGetRef(args[0]);
        for (auto &i: v.members) {
            insert(interpreter, {{i.first, stringMethods::getMethodsCxt()}, i.second}, passThisPtr);
        }
        return passThisPtr;
    }

    nativeFn(hashMapMethods::rexStr, interpreter, args, passThisPtr) {
        auto &container = passThisPtr->members[L"kvPairs"]->getLinkedList();
        std::wstringstream wss;
        wss << L'{';
        for (auto &i: container) {
            wss << globalMethods::stringify(interpreter, {*i->getVec()[1]}, {}).getStr()
                << L": " << globalMethods::stringify(interpreter, {*i->getVec()[2]}, {}).getStr() << L',';
        }
        if (!container.empty())
            wss.seekp(-1, wss.cur);
        wss << L'}';
        return {wss.str(), stringMethods::getMethodsCxt()};
    }

    nativeFn(hashMapMethods::insert, interpreter, args, passThisPtr) {
        auto hashTSize = passThisPtr->members[L"hashT"]->getVec().size();
        if (passThisPtr->members[L"kvPairs"]->getLinkedList().size() + 1 >= 2 * hashTSize) {
            realloc(interpreter, {(vint) (2 * hashTSize)}, passThisPtr);
        }
        auto &k = eleGetRef(args[0]);
        auto &v = args[1].isRef() ? args[1].getRef() : args[1];
        auto hashedKey = (vsize) (globalMethods::hash(interpreter, {k}, {}).basicValue.unknown);
        passThisPtr->members[L"kvPairs"]->getLinkedList().emplace_back(managePtr(value{value::vecObject{
                managePtr(value{(unknownPtr) hashedKey}), managePtr(k), managePtr(v)}, vecMethods::getMethodsCxt()}));
        auto kvPair = passThisPtr->members[L"kvPairs"]->getLinkedList().end();
        kvPair--;
        auto &bucket = passThisPtr->members[L"hashT"]->getVec()[hashedKey % hashTSize];
        if (!bucket) {
            bucket = managePtr(value{value::linkedListObject{}, linkedListMethods::getMethodsCxt()});
        }
        bucket->getLinkedList().push_back(managePtr(value{kvPair}));
        return passThisPtr;
    }

    nativeFn(hashMapMethods::realloc, interpreter, args, passThisPtr) {
        auto &newSize = args[0].isRef() ? args[0].getRef().getInt() : args[0].getInt();
        passThisPtr->members[L"hashT"]->getVec().clear();
        passThisPtr->members[L"hashT"]->getVec().resize(newSize);
        for (auto it = passThisPtr->members[L"kvPairs"]->getLinkedList().begin();
             it != passThisPtr->members[L"kvPairs"]->getLinkedList().end(); it++) {
            auto &kvPair = *it;
            auto &bucket = passThisPtr->members[L"hashT"]->getVec()[
                    (vsize) (kvPair->getVec()[0]->basicValue.unknown) %
                    newSize];
            if (!bucket) {
                bucket = managePtr(value{value::linkedListObject{}, linkedListMethods::getMethodsCxt()});
            }

            bucket->getLinkedList().push_back(managePtr(value{it}));
        }
        return passThisPtr;
    }

    nativeFn(hashMapMethods::remove, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        auto &key = eleGetRef(args[0]);
        auto hashedKey = (vsize) (globalMethods::hash(interpreter, {key}, {}).basicValue.unknown);
        auto &bucket = passThisPtr->members[L"hashT"]->getVec()[
                hashedKey % passThisPtr->members[L"hashT"]->getVec().size()];
        for (auto it = bucket->getLinkedList().begin(); it != bucket->getLinkedList().end(); ++it) {
            auto &kvPair = **(*it)->linkedListIterObj;
            if (in->opEqual(*kvPair->getVec()[1], key).getBool()) {
                passThisPtr->members[L"kvPairs"]->getLinkedList().erase(*(*it)->linkedListIterObj);
                bucket->getLinkedList().erase(it);
                break;
            }
        }
        return passThisPtr;
    }

    nativeFn(hashMapMethods::rexIndex, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        auto &key = eleGetRef(args[0]);
        auto hashedKey = (vsize) (globalMethods::hash(interpreter, {key}, {}).basicValue.unknown);
        auto &bucket = passThisPtr->members[L"hashT"]->getVec()[
                hashedKey % passThisPtr->members[L"hashT"]->getVec().size()];
        if (bucket->linkedListObj) {
            for (auto it = bucket->getLinkedList().begin(); it != bucket->getLinkedList().end(); ++it) {
                auto &kvPair = **(*it)->linkedListIterObj;
                if (in->opEqual(*kvPair->getVec()[1], key).getBool()) {
                    return kvPair->getVec()[2];
                }
            }
        }
        throw signalException(interpreter::makeErr(L"mappingError", L"undefined key: " + vstr{key}));
    }

    nativeFn(hashMapMethods::contains, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        auto &key = eleGetRef(args[0]);
        auto hashedKey = (vsize) (globalMethods::hash(interpreter, {key}, {}).basicValue.unknown);
        auto &bucket = passThisPtr->members[L"hashT"]->getVec()[
                hashedKey % passThisPtr->members[L"hashT"]->getVec().size()];
        if (bucket->linkedListObj) {
            for (auto it = bucket->getLinkedList().begin(); it != bucket->getLinkedList().end(); ++it) {
                auto &kvPair = **(*it)->linkedListIterObj;
                if (in->opEqual(*kvPair->getVec()[1], key).getBool()) {
                    return true;
                }
            }
        }
        return false;
    }

    nativeFn(hashMapMethods::rexIter, interpreter, args, passThisPtr) {
        return {iterator::getMethodsCxt(passThisPtr->members[L"kvPairs"]->getLinkedList())};
    }

    value::cxtObject hashMapMethods::iterator::getMethodsCxt(value::linkedListObject &container) {
        value::cxtObject result;
        result[L"container"] = managePtr(value{container, linkedListMethods::getMethodsCxt()});
        result[L"cur"] = managePtr(value{result[L"container"]->getLinkedList().begin()});
        result[L"next"] = managePtr(value{(value::nativeFuncPtr) next});
        return result;
    }

    // iterate over all kvPairs
    nativeFn(hashMapMethods::iterator::next, interpreter, args, passThisPtr) {
        auto &container = passThisPtr->members[L"container"]->getLinkedList();
        auto &iter = passThisPtr->members[L"cur"]->linkedListIterObj;
        auto &element = **iter;
        if (*iter == container.end()) {
            return interpreter::makeIt({}, true);
        }
        (*iter)++;

        return interpreter::makeIt(element->isRef() ? element->refObj : element, true);
    }

    nativeFn(hashMapMethods::keys, interpreter, args, passThisPtr) {
        value::vecObject vec;
        for (auto &i: passThisPtr->members[L"kvPairs"]->getLinkedList()) {
            vec.push_back(i->getVec()[1]);
        }
        return {vec, vecMethods::getMethodsCxt()};
    }

    value::cxtObject hashMapMethods::keysIterator::getMethodsCxt(value::linkedListObject &container) {
        value::cxtObject result;
        result[L"container"] = managePtr(value{container, linkedListMethods::getMethodsCxt()});
        result[L"cur"] = managePtr(value{result[L"container"]->getLinkedList().begin()});
        result[L"next"] = managePtr(value{(value::nativeFuncPtr) next});
        return result;
    }

    // iterate over all kvPairs and get the keys
    nativeFn(hashMapMethods::keysIterator::next, interpreter, args, passThisPtr) {
        auto &container = passThisPtr->members[L"container"]->getLinkedList();
        auto &iter = passThisPtr->members[L"cur"]->linkedListIterObj;
        auto &element = **iter;
        if (*iter == container.end()) {
            throw signalBreak();
        }
        (*iter)++;

        return *element->getVec()[1];
    }

    nativeFn(hashMapMethods::rexClone, interpreter, args, passThisPtr) {
        managedPtr<value> result = managePtr(value{});
        passThisPtr->deepCopy(*result);
        realloc(interpreter, {(unknownPtr) passThisPtr->members[L"hashT"]->getVec().size()}, result);
        return result;
    }

    nativeFn(globalMethods::hashMap, interpreter, args, passThisPtr) {
        return {hashMapMethods::getMethodsCxt()};
    }

    value::cxtObject iterMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"forEach"] = managePtr(value{value::nativeFuncPtr{forEach}});
        result[L"collect"] = managePtr(value{value::nativeFuncPtr{collect}});
        return result;
    }

    nativeFn(iterMethods::collect, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);

        std::shared_ptr<value> obj = args[0].isRef() ? args[0].refObj : managePtr(args[0]);

        std::shared_ptr<value> rexIter;
        if (auto it = obj->members.find(L"rexIter"); it != obj->members.end()) {
            rexIter = it->second;
        } else {
            throw signalException(interpreter::makeErr(L"internalError", L"undefined identifier: `rexIter`"));
        }

        std::shared_ptr<value> rIter = managePtr(in->invokeFunc(rexIter, {}, obj));
        if (rIter->isRef())
            rIter = rIter->refObj;

        auto cxtIdx = in->stack.back().getCurLocalCxtIdx();

        std::shared_ptr<value> itNext;

        if (auto it = rIter->members.find(L"next"); it != rIter->members.end())
            itNext = it->second;
        else
            throw signalException(interpreter::makeErr(L"internalError", L"undefined identifier: `next`"));

        value result{value::vecObject{}, vecMethods::getMethodsCxt()};

        while (true) {
            if (auto val = in->invokeFunc(itNext, {}, rIter); val.getVec()[1]->getBool()) {
                in->stack.back().backToLocalCxt(cxtIdx);
                break;
            } else
                result.getVec().push_back(val.getVec()[0]->isRef() ? val.getVec()[0]->refObj : val.getVec()[0]);
        }

        return result;
    }

    nativeFn(iterMethods::forEach, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);

        auto obj = args[0].isRef() ? args[0].refObj : managePtr(args[0]);
        auto callback = args[1].isRef() ? args[1].refObj : managePtr(args[1]);

        std::shared_ptr<value> rexIter;
        if (auto it = obj->members.find(L"rexIter"); it != obj->members.end()) {
            rexIter = it->second;
        } else {
            throw signalException(interpreter::makeErr(L"internalError", L"undefined identifier: `rexIter`"));
        }

        std::shared_ptr<value> rIter = managePtr(in->invokeFunc(rexIter, {}, obj));
        if (rIter->isRef())
            rIter = rIter->refObj;

        auto cxtIdx = in->stack.back().getCurLocalCxtIdx();

        managedPtr<value> itNext, itVal;

        if (auto it = rIter->members.find(L"next"); it != rIter->members.end())
            itNext = it->second;
        else
            throw signalException(interpreter::makeErr(L"internalError", L"undefined identifier: `next`"));

        value result{value::vecObject{}, vecMethods::getMethodsCxt()};

        while (true) {
            if (auto val = in->invokeFunc(itNext, {}, rIter); val.getVec()[1]->getBool()) {
                in->stack.back().backToLocalCxt(cxtIdx);
                break;
            } else
                itVal = val.getVec()[0]->isRef() ? val.getVec()[0]->refObj : val.getVec()[0];

            in->invokeFunc(callback, {itVal}, {});
        }

        return result;
    }

    nativeFn(objectMethods::iterate, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        auto callback = args[1].isRef() ? args[1].refObj : managePtr(args[1]);
        for (auto &i: args[0].isRef() ? args[0].getRef().members : args[0].members) {
            in->invokeFunc(callback, {{i.first, stringMethods::getMethodsCxt()}, i.second}, {});
        }
        return {};
    }

    nativeFn(objectMethods::addAttr, interpreter, args, passThisPtr) {
        args[0].refObj->members[eleGetRef(args[1]).getStr()] = eleRefObj(args[2]);
        return args[0];
    }

    nativeFn(objectMethods::removeAttr, interpreter, args, passThisPtr) {
        args[0].refObj->members.erase(eleGetRef(args[1]).getStr());
        return args[0];
    }

    nativeFn(objectMethods::contains, interpreter, args, passThisPtr) {
        return args[0].refObj->members.contains(eleGetRef(args[1]).getStr());
    }

    value::cxtObject objectMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"iterate"] = managePtr(value{value::nativeFuncPtr{iterate}});
        result[L"addAttr"] = managePtr(value{value::nativeFuncPtr{addAttr}});
        result[L"removeAttr"] = managePtr(value{value::nativeFuncPtr{removeAttr}});
        result[L"contains"] = managePtr(value{value::nativeFuncPtr{contains}});
        return result;
    }

    namespace mutexMethods {
        value::cxtObject getMethodsCxt(std::mutex *ptr) {
            value::cxtObject result;
            result[L"rexInit"] = managePtr(value{value::nativeFuncPtr{rexInit}});
            result[L"lock"] = managePtr(value{value::nativeFuncPtr{lock}});
            result[L"tryLock"] = managePtr(value{value::nativeFuncPtr{tryLock}});
            result[L"rexFree"] = managePtr(value{value::nativeFuncPtr{rexFree}});
            result[L"unlock"] = managePtr(value{value::nativeFuncPtr{unlock}});
            result[L"finalize"] = managePtr(value{value::nativeFuncPtr{finalize}});
            result[L"__lock__"] = managePtr(value{(unknownPtr) ptr});
            return result;
        }

        nativeFn(rexInit, interpreter, args, passThisPtr) {
            auto lock = (std::mutex *) passThisPtr->members[L"__lock__"]->basicValue.unknown;
            try {
                lock->lock();
            } catch (std::system_error &e) {
                throw signalException(interpreter::makeErr(
                        L"mutexError", L"[Error " + std::to_wstring(e.code().value())
                                       + L"] " + string2wstring(e.what())));
            }
            return {};
        }

        nativeFn(lock, interpreter, args, passThisPtr) {
            auto lock = (std::mutex *) passThisPtr->members[L"__lock__"]->basicValue.unknown;
            try {
                lock->lock();
            } catch (std::system_error &e) {
                throw signalException(interpreter::makeErr(
                        L"mutexError", L"[Error " + std::to_wstring(e.code().value())
                                       + L"] " + string2wstring(e.what())));
            }
            return {};
        }

        nativeFn(tryLock, interpreter, args, passThisPtr) {
            auto lock = (std::mutex *) passThisPtr->members[L"__lock__"]->basicValue.unknown;
            return lock->try_lock();
        }

        nativeFn(unlock, interpreter, args, passThisPtr) {
            auto lock = (std::mutex *) passThisPtr->members[L"__lock__"]->basicValue.unknown;
            lock->unlock();
            return {};
        }

        nativeFn(rexFree, interpreter, args, passThisPtr) {
            auto lock = (std::mutex *) passThisPtr->members[L"__lock__"]->basicValue.unknown;
            lock->unlock();
            return {};
        }

        nativeFn(finalize, interpreter, args, passThisPtr) {
            auto lock = (std::mutex *) passThisPtr->members[L"__lock__"]->basicValue.unknown;
            delete lock;
            return {};
        }
    }
}
#pragma clang diagnostic pop