//
// Created by XIaokang00010 on 2022/12/3.
//

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

    value stringMethods::substr(void *interpreter, vec<value> args, managedPtr<value> passThisPtr) {
        return {};
    }

    value stringMethods::startsWith(void *interpreter, vec<value> args, managedPtr<value> passThisPtr) {
        return {};
    }

    value stringMethods::endsWith(void *interpreter, vec<value> args, managedPtr<value> passThisPtr) {
        return {};
    }

    value stringMethods::charAt(void *interpreter, vec<value> args, managedPtr<value> passThisPtr) {
        return {};
    }

    value stringMethods::fromChar(void *interpreter, vec<value> args, managedPtr<value> passThisPtr) {
        return {};
    }

    value stringMethods::length(void *interpreter, vec<value> args, managedPtr<value> passThisPtr) {
        return {};
    }

    value::cxtObject vecMethods::getMethodsCxt() {
        return {};
    }
}