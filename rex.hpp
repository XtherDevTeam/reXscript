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
    managedPtr<value> importExternModule(const managedPtr<environment> &env, const vstr &path);

    /**
     * @brief Import a native module
     * @param env The environment object for the current interpreter
     * @param path The path to the native module file to import
     * @return A pointer to the module context object
     */
    managedPtr<value> importNativeModule(const managedPtr<environment> &env, const vstr &path);

    /**
     * @brief Import an external reXscript module without importPathPrefix
     * @param env The environment object for the current interpreter
     * @param path The path to the module file to import
     * @return A pointer to the module context object
     */
    managedPtr<value> importExternModuleEx(const managedPtr<environment> &env, const vstr &fullPath);

    /**
     * @brief Import a native module without importPathPrefix
     * @param env The environment object for the current interpreter
     * @param path The path to the native module file to import
     * @return A pointer to the module context object
     */
    managedPtr<value> importNativeModuleEx(const managedPtr<environment> &env, const vstr &fullPath);
}

#endif //REXSCRIPT_REX_HPP
