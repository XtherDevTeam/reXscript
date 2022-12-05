//
// Created by XIaokang00010 on 2022/12/3.
//

#include <iostream>
#include "builtInMethods.hpp"

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
        return {};
    }

    value stringMethods::startsWith(void *selfPtr, vec<value> args, const managedPtr<value>&  passThisPtr) {
        return {};
    }

    value stringMethods::endsWith(void *selfPtr, vec<value> args, const managedPtr<value>&  passThisPtr) {
        return {};
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
        return result;
    }
}