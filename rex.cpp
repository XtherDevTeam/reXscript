//
// Created by XIaokang00010 on 2022/12/5.
//

#include <fstream>
#include <share/dlfcn.hpp>
#include "rex.hpp"
#include "exceptions/signalException.hpp"
#include "exceptions/endOfFileException.hpp"

namespace rex {

    rex::managedPtr<rex::environment> getRexEnvironment() {
        return rex::managePtr(rex::environment{managePtr(value{globalMethods::getMethodsCxt()})});
    }

    managedPtr<value> importExternModules(const managedPtr<environment> &env, const vstr &path) {
        // 获取 importPrefixPath 向量
        if (auto it = env->globalCxt->members.find(L"importPrefixPath"); it == env->globalCxt->members.end()) {
            throw signalException(interpreter::makeErr(L"referenceError", L"importPrefixPath not found"));
        } else {
            vec<managedPtr<value>> &importPrefixPath = it->second->getVec();
            // 遍历 importPrefixPath 向量中的所有字符串
            for (const auto &prefixPath: importPrefixPath) {
                // 获取字符串对象
                vstr fullPath = prefixPath->isRef() ? prefixPath->getRef().getStr() : prefixPath->getStr();

                path::join(fullPath, path);

                if (it = env->globalCxt->members.find(fullPath); it != env->globalCxt->members.end()) {
                    return it->second;
                } else {
                    std::wifstream f(wstring2string(fullPath), std::ios::in);
                    if (f.is_open()) {
                        auto moduleCxt = rex::managePtr(rex::value{rex::value::cxtObject{}});
                        env->globalCxt->members[path] = moduleCxt;
                        moduleCxt->members[L"__path__"] = managePtr(
                                value{fullPath, rex::stringMethods::getMethodsCxt()});
                        rex::managedPtr<rex::interpreter> interpreter = managePtr(rex::interpreter{env, moduleCxt});
                        interpreter->interpreterCxt[L"thread_id"] = rex::managePtr(rex::value{(rex::vint) 0});
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
                        continue;
                    }
                }
            }
            throw signalException(
                    interpreter::makeErr(L"fileError", L"Cannot open file: file not exist or damaged"));
        }
    }

    managedPtr<value> importNativeModules(const managedPtr<environment> &env, const vstr &path) {
        // Thanks for AI's help
        // 获取 importPrefixPath 向量
        if (auto it = env->globalCxt->members.find(L"importPrefixPath"); it == env->globalCxt->members.end()) {
            throw signalException(interpreter::makeErr(L"referenceError", L"importPrefixPath not found"));
        } else {
            vec<managedPtr<value>> &importPrefixPath = it->second->getVec();
            // 遍历 importPrefixPath 向量中的所有字符串
            for (const auto &prefixPath: importPrefixPath) {
                // 获取字符串对象
                vstr fullPath = prefixPath->isRef() ? prefixPath->getRef().getStr() : prefixPath->getStr();

                path::join(fullPath, path);
                void *handle = dlopen(wstring2string(fullPath).c_str(), RTLD_LAZY);
                if (!handle)
                    continue;

                rex::managedPtr<rex::value> moduleCxt = rex::managePtr(rex::value{rex::value::cxtObject{}});
                env->globalCxt->members[path] = moduleCxt;
                moduleCxt->members[L"__path__"] = managePtr(
                        value{fullPath, rex::stringMethods::getMethodsCxt()});
                moduleCxt->members[L"__handle__"] = managePtr(value{(unknownPtr) handle});

                using funcPtr = void(const managedPtr<environment> &, const managedPtr<value> &);

                std::function<funcPtr> rexModInit = (funcPtr *) (dlsym(handle, "rexModInit"));

                rexModInit(env, moduleCxt);

                return moduleCxt;
            }

            throw signalException(
                    interpreter::makeErr(L"fileError", L"Cannot open file: file not exist or damaged"));
        }
    }
}