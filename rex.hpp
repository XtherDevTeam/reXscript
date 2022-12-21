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
    /**
     * @brief Get the environment object for the current reXscript interpreter
     * @return A pointer to the environment object
     */
    rex::managedPtr<rex::environment> getRexEnvironment();

    /**
     * @brief Import an external reXscript module
     * @param env The environment object for the current interpreter
     * @param path The path to the module file to import
     * @return A pointer to the module context object
     */
    managedPtr<value> importExternModules(const managedPtr<environment> &env, const vstr &path);

    /**
     * @brief Import a native module
     * @param env The environment object for the current interpreter
     * @param path The path to the native module file to import
     * @return A pointer to the module context object
     */
    managedPtr<value> importNativeModules(const managedPtr<environment> &env, const vstr &path);
}

#endif //REXSCRIPT_REX_HPP
