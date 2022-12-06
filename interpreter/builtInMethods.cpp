//
// Created by XIaokang00010 on 2022/12/3.
//

#include <iostream>
#include "builtInMethods.hpp"
#include "rex.hpp"
#include "exceptions/signalException.hpp"

namespace rex {
    value::cxtObject stringMethods::getMethodsCxt() {
        value::cxtObject result;
        result[L"substr"] = managePtr(value{(value::nativeFuncPtr) substr});
        result[L"startsWith"] = managePtr(value{(value::nativeFuncPtr) startsWith});
        result[L"endsWith"] = managePtr(value{(value::nativeFuncPtr) endsWith});
        result[L"charAt"] = managePtr(value{(value::nativeFuncPtr) charAt});
        result[L"fromChar"] = managePtr(value{(value::nativeFuncPtr) fromChar});
        result[L"length"] = managePtr(value{(value::nativeFuncPtr) length});
        return result;
    }

    value stringMethods::substr(void *selfPtr, vec<value> args, const managedPtr<value>&  passThisPtr) {
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
                throw rex::signalException(rex::interpreter::makeErr(L"argumentsError", L"substr() expected one or two arguments"));
            }
        }
    }

    value stringMethods::startsWith(void *selfPtr, vec<value> args, const managedPtr<value>&  passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();
        return {passThisPtr->getStr().starts_with(args[0].getStr())};
    }

    value stringMethods::endsWith(void *selfPtr, vec<value> args, const managedPtr<value>&  passThisPtr) {
        if (args[0].isRef())
            args[0] = args[0].getRef();
        return {passThisPtr->getStr().ends_with(args[0].getStr())};
    }

    value stringMethods::charAt(void *selfPtr, vec<value> args, const managedPtr<value>&  passThisPtr) {
        return {(vint) passThisPtr->getStr()[args[0].getInt()]};
    }

    value stringMethods::fromChar(void *selfPtr, vec<value> args, const managedPtr<value>&  passThisPtr) {
        passThisPtr->getStr() = (vchar)args[0].getInt();
    }

    value stringMethods::length(void *selfPtr, vec<value> args, const managedPtr<value>&  passThisPtr) {
        return {(vint) passThisPtr->getStr().length()};
    }

    value::cxtObject vecMethods::getMethodsCxt() {
        return {};
    }

    value globalMethods::input(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr) {
        std::string s;
        std::getline(std::cin, s);
        return {string2wstring(s), rex::stringMethods::getMethodsCxt()};
    }

    value globalMethods::print(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr) {
        for (auto &i : args) {
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
        return result;
    }

    value globalMethods::rexImport(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr) {
        auto *in = (rex::interpreter*)interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return rex::importExternModules(in->env, args[0].getStr());
    }

    value globalMethods::rexNativeImport(void *interpreter, vec<value> args, const managedPtr<value> &passThisPtr) {
        auto *in = (rex::interpreter*)interpreter;
        if (args[0].isRef())
            args[0] = args[0].getRef();

        return rex::importNativeModules(in->env, args[0].getStr());
    }
}