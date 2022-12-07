//
// Created by XIaokang00010 on 2022/12/5.
//

#ifndef REXSCRIPT_REX_HPP
#define REXSCRIPT_REX_HPP

#include <iostream>
#include <frontend/ast.hpp>
#include <frontend/parser.hpp>
#include <pass/stringToLexerPass.hpp>
#include "interpreter/value.hpp"
#include "interpreter/builtInMethods.hpp"
#include "interpreter/interpreter.hpp"

namespace rex {
    rex::managedPtr<rex::environment> getRexEnvironment();

    managedPtr<value> importExternModules(const managedPtr<environment> &env, const vstr &path);

    managedPtr<value> importNativeModules(const managedPtr<environment> &env, const vstr &path);

    void
    rexThreadWrapper(const managedPtr<environment> &env, const managedPtr<value> &cxt, const managedPtr<value> &func,
                     const vec<value> &args,
                     managedPtr<value> passThisPtr = nullptr);

    void spawnThread(const managedPtr<environment> &env, const managedPtr<value> &cxt, const managedPtr<value> &func,
                     const vec<value> &args, managedPtr<value> passThisPtr = nullptr);
}

#endif //REXSCRIPT_REX_HPP
