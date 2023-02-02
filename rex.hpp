//
// Created by XIaokang00010 on 2022/12/5.
//

#ifndef REXSCRIPT_REX_HPP
#define REXSCRIPT_REX_HPP

#include <iostream>
#include <frontend/ast.hpp>
#include <frontend/parser.hpp>
#include "interpreter/value.hpp"
#include "interpreter/builtInMethods.hpp"
#include "interpreter/interpreter.hpp"

namespace rex {
    /**
     * @brief Get the environment object for the current reXscript interpreter
     * @return A pointer to the environment object
     */
    rex::managedPtr<rex::environment> getRexEnvironment();

    rex::managedPtr<rex::interpreter> getRexInterpreter();

    /**
     * @brief Import an external reXscript module
     * @param interpreter The environment object for the current interpreter
     * @param path The path to the module file to import
     * @return A pointer to the module context object
     */
    managedPtr <value>
    importExternModule(interpreter *interpreter, const vstr &path);

    /**
     * @brief Import a native module
     * @param interpreter The interpreter
     * @param path The path to the native module file to import
     * @return A pointer to the module context object
     */
    managedPtr <value>
    importNativeModule(interpreter *interpreter, const vstr &path);

    /**
     * @brief Import an external reXscript package
     * @param interpreter The interpreter
     * @param path The path to the package directory to import
     * @return A pointer to the module context object
     */
    managedPtr<value> importExternPackage(interpreter *interpreter, const vstr &pkgName);

    /**
     * @brief Import an external reXscript module without importPathPrefix
     * @param interpreter The interpreter
     * @param path The path to the module file to import
     * @return A pointer to the module context object
     */
    managedPtr <value>
    importExternModuleEx(interpreter *interpreter, const vstr &fullPath);

    /**
     * @brief Import a native module without importPathPrefix
     * @param interpreter The interpreter
     * @param path The path to the native module file to import
     * @return A pointer to the module context object
     */
    managedPtr <value> importNativeModuleEx(interpreter *interpreter, const vstr &fullPath);

    /**
     * @brief Import an external package without importPathPrefix
     * @param interpreter The interpreter
     * @param pkgDirPath The path to the package file to import
     * @return A pointer to the package context object
     */
    managedPtr<value>
    importExternPackageEx(interpreter *interpreter, const vstr &pkgDirPath);

    managedPtr <value>
    importEx(interpreter *interpreter, const vstr &modPath);

    AST getFileAST(const vstr &path);

    void atExitHandler();

    void atExitWrapper(int sig);

    void initializeAtExitHandler();

    void initialize();
}

#endif //REXSCRIPT_REX_HPP
