//
// Created by XIaokang00010 on 2022/12/5.
//

#include <fstream>
#include "rex.hpp"
#include "exceptions/signalException.hpp"
#include "exceptions/endOfFileException.hpp"

namespace rex {

    rex::managedPtr<rex::environment> getRexEnvironment() {
        return rex::managePtr(rex::environment{});
    }

    managedPtr<value> importExternModules(const managedPtr<environment> &env, const vstr &path) {
        if (auto it = env->globalCxt->members.find(path); it != env->globalCxt->members.end()) {
            return it->second;
        } else {
            std::wifstream f(path, std::ios::in);
            if (f.is_open()) {
                rex::managedPtr<rex::value> moduleCxt = rex::managePtr(rex::value{});
                env->globalCxt->members[path] = moduleCxt;
                moduleCxt->members[L"__path__"] = managePtr(
                        value{path, rex::stringMethods::getMethodsCxt()});
                rex::managedPtr<rex::interpreter> interpreter = managePtr(rex::interpreter{env, moduleCxt});
                rex::lexer lexer{f};
                rex::parser parser{lexer};
                rex::AST ast = parser.parseFile();
                for (auto &i: ast.child) {
                    interpreter->interpret(i);
                }
                if (auto vit = moduleCxt->members.find(L"rexModInit"); vit != moduleCxt->members.end()) {
                    interpreter->invokeFunc(vit->second, {}, {});
                }
                return moduleCxt;
            } else {
                throw signalException(interpreter::makeErr(L"fileError", L"Cannot open file: `" + path + L"`"));
            }
        }
    }
}