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
        rex::split(passThisPtr->getStr(), args[0].getStr(), [&](const vstr &r) {
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
        auto in = static_cast<rex::interpreter *>(interpreter);
        vstr &charsetName = args[0].isRef() ? args[0].getRef().getStr() : args[0].getStr();
        auto encoder = in->env->globalCxt->members[L"charsets"]->members[charsetName]->members[L"encoder"];
        return in->invokeFunc(encoder, {passThisPtr}, {});
    }

    value::cxtObject vecMethods::getMethodsCxt() {
        value::cxtObject result;
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

    nativeFn(vecMethods::append, interpreter, args, passThisPtr) {
        for (auto &i: args)
            passThisPtr->getVec().push_back(managePtr(i.isRef() ? i.getRef() : i));
        return passThisPtr;
    }

    nativeFn(vecMethods::pop, interpreter, args, passThisPtr) {
        passThisPtr->getVec().pop_back();
        return passThisPtr;
    }

    nativeFn(vecMethods::remove, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        value lhs = args[0].isRef() ? args[0].getRef() : args[0];
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
        value lhs = args[0].isRef() ? args[0].getRef() : args[0];
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

    nativeFn(globalMethods::input, interpreter, args, passThisPtr) {
        std::string s;
        std::getline(std::cin, s);
        return {string2wstring(s), rex::stringMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::print, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        for (auto &item: args) {
            auto &i = item.isRef() ? item.getRef() : item;
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
                case value::vKind::vBytes: {
                    std::ios::fmtflags f(std::cout.flags());
                    for (auto &c: i.getBytes()) {
                        if (isprint(c)) {
                            std::cout << c;
                        } else {
                            std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                                      << static_cast<int>((unsigned char) c);
                        }
                    }
                    std::cout.flags(f);
                    break;
                }
                default:
                    if (auto it = i.members.find(L"rexStr"); it != i.members.end()) {
                        std::cout << wstring2string(
                                in->invokeFunc(it->second, {}, item.isRef() ? item.refObj : managePtr(item)).getStr());
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
        value &element = args[0].isRef() ? args[0].getRef() : args[0];
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

    value::cxtObject globalMethods::getMethodsCxt() {
        value::cxtObject result;
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

        result[L"threading"] = managePtr(threadingMethods::getThreadingModule());
        result[L"importPrefixPath"] = managePtr(value{value::vecObject{
                managePtr(value{L"", stringMethods::getMethodsCxt()})
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
        vstr &str = args[0].isRef() ? args[0].getRef().getStr() : args[0].getStr();
        vbytes output{};

        for (auto &i: str) {
            output.push_back(i < 256 ? (vbyte) i : '?');
        }

        return {output, bytesMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::charsetsMethods::ansiDecoder, interpreter, args, passThisPtr) {
        vbytes &str = args[0].isRef() ? args[0].getRef().getBytes() : args[0].getBytes();
        vstr output{};

        for (auto &i: str) {
            output += i;
        }

        return {output, stringMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::charsetsMethods::utf8Encoder, interpreter, args, passThisPtr) {
        vstr &str = args[0].isRef() ? args[0].getRef().getStr() : args[0].getStr();
        vbytes output{wstring2string(str)};

        return {output, bytesMethods::getMethodsCxt()};
    }

    nativeFn(globalMethods::charsetsMethods::utf8Decoder, interpreter, args, passThisPtr) {
        vbytes &str = args[0].isRef() ? args[0].getRef().getBytes() : args[0].getBytes();
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

        return rex::importExternModule(in->env, args[0].getStr(), defCxt);
    }

    nativeFn(globalMethods::rexRequireNativeMod, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        value::cxtObject defCxt;
        if (auto it = in->moduleCxt->members.find(L"rexPkgRoot"); it != in->moduleCxt->members.end()) {
            defCxt.insert(*it);
        }

        return rex::importNativeModule(in->env, args[0].getStr(), defCxt);
    }

    nativeFn(globalMethods::rexRequirePackage, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return rex::importExternPackage(in->env, args[0].getStr());
    }

    nativeFn(globalMethods::rexRequire, interpreter, args, passThisPtr) {
        auto *in = (rex::interpreter *) interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        value::cxtObject defCxt;
        if (auto it = in->moduleCxt->members.find(L"rexPkgRoot"); it != in->moduleCxt->members.end()) {
            defCxt.insert(*it);
        }

        return importEx(in->env, args[0].getStr(), defCxt);
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
        vstr &charsetName = args[0].isRef() ? args[0].getRef().getStr() : args[0].getStr();
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
            throw signalBreak();
        auto res = container[index]->isRef() ? container[index]->refObj : container[index];
        index++;
        return {res};
    }

    value::cxtObject linkedListMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"append"] = managePtr(value{(value::nativeFuncPtr) append});
        result[L"pop"] = managePtr(value{(value::nativeFuncPtr) pop});
        result[L"remove"] = managePtr(value{(value::nativeFuncPtr) remove});
        result[L"removeAll"] = managePtr(value{(value::nativeFuncPtr) removeAll});
        result[L"rexIter"] = managePtr(value{(value::nativeFuncPtr) rexIter});
        return result;
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
        value lhs = args[0].isRef() ? args[0].getRef() : args[0];
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
        value lhs = args[0].isRef() ? args[0].getRef() : args[0];
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
        result[L"cur"] = managePtr(value{container.begin()});
        result[L"next"] = managePtr(value{(value::nativeFuncPtr) next});
        return result;
    }

    nativeFn(linkedListMethods::iterator::next, interpreter, args, passThisPtr) {
        auto &container = passThisPtr->members[L"container"]->getLinkedList();
        auto &iter = passThisPtr->members[L"cur"]->linkedListIterObj;
        auto &element = **iter;
        if (*iter == container.end()) {
            throw signalBreak();
        }
        (*iter)++;

        return element->isRef() ? element->refObj : *element;
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
        // Members
        result[L"kvPairs"] = managePtr(value{value::linkedListObject{}, linkedListMethods::getMethodsCxt()});
        result[L"hashT"] = managePtr(value{value::vecObject{}, vecMethods::getMethodsCxt()});
        result[L"hashT"]->getVec().resize(defaultHashTSize);
        return result;
    }

    nativeFn(hashMapMethods::insert, interpreter, args, passThisPtr) {
        auto hashTSize = passThisPtr->members[L"hashT"]->getVec().size();
        if (passThisPtr->members[L"kvPairs"]->getLinkedList().size() + 1 >= 2 * hashTSize) {
            realloc(interpreter, {(vint) (2 * hashTSize)}, passThisPtr);
        }
        auto &k = args[0].isRef() ? args[0].getRef() : args[0];
        auto &v = args[1].isRef() ? args[1].getRef() : args[1];
        auto hashedKey = (vsize) (globalMethods::hash(interpreter, {k}, {}).basicValue.unknown);
        passThisPtr->members[L"kvPairs"]->getLinkedList().emplace_back(managePtr(value{value::vecObject{
                managePtr(value{(unknownPtr) hashedKey}), managePtr(k), managePtr(v)}, vecMethods::getMethodsCxt()}));
        auto kvPair = passThisPtr->members[L"kvPairs"]->getLinkedList().end()--;
        auto &bucket = passThisPtr->members[L"hashT"]->getVec()[hashedKey % hashTSize];
        if (bucket->kind == value::vKind::vNull) {
            bucket = managePtr(value{value::linkedListObject{}, linkedListMethods::getMethodsCxt()});
        }
        bucket->getLinkedList().push_back(managePtr(value{kvPair}));
        return passThisPtr;
    }

    nativeFn(hashMapMethods::realloc, interpreter, args, passThisPtr) {
        auto &newSize = args[0].isRef() ? args[0].getRef().getInt() : args[0].getInt();
        passThisPtr->members[L"hashT"]->getVec().clear();
        passThisPtr->members[L"hashT"]->getVec().resize(newSize);
        for (auto &kvPair: passThisPtr->members[L"kvPairs"]->getLinkedList()) {
            auto &bucket = (**passThisPtr->members[L"hashT"]->linkedListIterObj)->getVec()[
                    (vsize) (kvPair->getVec()[0]->basicValue.unknown) %
                    newSize];
            if (bucket->kind == value::vKind::vNull) {
                bucket = managePtr(value{value::linkedListObject{}, linkedListMethods::getMethodsCxt()});
            }

            bucket->getLinkedList().push_back(managePtr(value{kvPair}));
        }
        return passThisPtr;
    }

    nativeFn(hashMapMethods::remove, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        auto &key = args[0].isRef() ? args[0].getRef() : args[0];
        auto hashedKey = (vsize) (globalMethods::hash(interpreter, {key}, {}).basicValue.unknown);
        auto &bucket = passThisPtr->members[L"hashT"]->getVec()[
                hashedKey % passThisPtr->members[L"hashT"]->getVec().size()];
        for (auto it = bucket->getLinkedList().begin(); it != bucket->getLinkedList().end(); ++it) {
            auto &kvPair = **(*it)->linkedListIterObj;
            if (in->opEqual(*kvPair->getVec()[1], key).getBool()) {
                bucket->getLinkedList().erase(it);
                passThisPtr->members[L"kvPairs"]->getLinkedList().erase(*(*it)->linkedListIterObj);
                break;
            }
        }
        return passThisPtr;
    }

    nativeFn(hashMapMethods::rexIndex, interpreter, args, passThisPtr) {
        auto in = static_cast<rex::interpreter *>(interpreter);
        auto &key = args[0].isRef() ? args[0].getRef() : args[0];
        auto hashedKey = (vsize) (globalMethods::hash(interpreter, {key}, {}).basicValue.unknown);
        auto &bucket = passThisPtr->members[L"hashT"]->getVec()[
                hashedKey % passThisPtr->members[L"hashT"]->getVec().size()];
        for (auto it = bucket->getLinkedList().begin(); it != bucket->getLinkedList().end(); ++it) {
            auto &kvPair = **(*it)->linkedListIterObj;
            if (in->opEqual(*kvPair->getVec()[1], key).getBool()) {
                return kvPair->getVec()[2];
            }
        }
        throw signalException(interpreter::makeErr(L"mappingError", L"undefined key: " + vstr{key}));
    }

    nativeFn(hashMapMethods::rexIter, interpreter, args, passThisPtr) {
        return {iterator::getMethodsCxt(passThisPtr->getLinkedList())};
    }

    value::cxtObject hashMapMethods::iterator::getMethodsCxt(value::linkedListObject &container) {
        value::cxtObject result;
        result[L"container"] = managePtr(value{container, linkedListMethods::getMethodsCxt()});
        result[L"cur"] = managePtr(value{container.begin()});
        result[L"next"] = managePtr(value{(value::nativeFuncPtr) next});
        return result;
    }

    // iterate over all kvPairs
    nativeFn(hashMapMethods::iterator::next, interpreter, args, passThisPtr) {
        auto &container = passThisPtr->members[L"container"]->getLinkedList();
        auto &iter = passThisPtr->members[L"cur"]->linkedListIterObj;
        auto &element = **iter;
        if (*iter == container.end()) {
            throw signalBreak();
        }
        (*iter)++;

        return element->isRef() ? element->refObj : *element;
    }

    nativeFn(hashMapMethods::keys, interpreter, args, passThisPtr) {
        return {iterator::getMethodsCxt(passThisPtr->getLinkedList())};
    }

    value::cxtObject hashMapMethods::keysIterator::getMethodsCxt(value::linkedListObject &container) {
        value::cxtObject result;
        result[L"container"] = managePtr(value{container, linkedListMethods::getMethodsCxt()});
        result[L"cur"] = managePtr(value{container.begin()});
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

    nativeFn(globalMethods::hashMap, interpreter, args, passThisPtr) {
        return {hashMapMethods::getMethodsCxt()};
    }
}
#pragma clang diagnostic pop