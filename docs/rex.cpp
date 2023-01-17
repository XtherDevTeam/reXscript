//
// Created by XIaokang00010 on 2022/12/5.
//

#include <fstream>
#include "share/dlfcn.hpp"
#include "rex.hpp"
#include "exceptions/signalException.hpp"
#include "exceptions/endOfFileException.hpp"
#include "exceptions/importError.hpp"

namespace rex {

    rex::managedPtr<rex::environment> getRexEnvironment() {
        auto env = rex::managePtr(rex::environment{managePtr(value{globalMethods::getMethodsCxt()})});
        auto cstr = std::getenv("rexPackagesPath");
        auto str = rex::string2wstring(cstr == nullptr ? "" : cstr);
        managedPtr<value> rexPackagesPath = managePtr(value{value::vecObject{}, vecMethods::getMethodsCxt()});
        if (!str.empty()) {
            split(str, vstr{L";"}, [&](const vstr &i, vsize) {
                rexPackagesPath->getVec().push_back(
                        managePtr(value{i, rex::stringMethods::getMethodsCxt()}));
            });
        }
        env->globalCxt->members[L"rexPackagesPath"] = rexPackagesPath;
        env->globalCxt->members[L"rexArgs"] = managePtr(value{value::vecObject{}, vecMethods::getMethodsCxt()});
        return env;
    }

    managedPtr <value>
    importExternModule(interpreter *interpreter, const vstr &path) {
        // 获取 importPrefixPath 向量
        if (auto it = interpreter->env->globalCxt->members.find(L"importPrefixPath"); it == interpreter->env->globalCxt->members.end()) {
            throw signalException(interpreter::makeErr(L"internalError", L"importPrefixPath not found"));
        } else {
            vec<managedPtr<value>> &importPrefixPath = it->second->getVec();
            // 遍历 importPrefixPath 向量中的所有字符串
            for (const auto &prefixPath: importPrefixPath) {
                // 获取字符串对象
                vstr fullPath = prefixPath->isRef() ? prefixPath->getRef().getStr() : prefixPath->getStr();

                path::join(fullPath, path);
                fullPath = path::getRealpath(fullPath);

                try {
                    return importExternModuleEx(interpreter, fullPath);
                } catch (rex::importError &e) {
                    continue;
                }
            }
            throw importError(L"Cannot open file: file not exist or damaged");
        }
    }

    managedPtr <value>
    importNativeModule(interpreter *interpreter, const vstr &path) {
        // Thanks for AI's help
        // 获取 importPrefixPath 向量
        if (auto it = interpreter->env->globalCxt->members.find(L"importPrefixPath"); it == interpreter->env->globalCxt->members.end()) {
            throw signalException(interpreter::makeErr(L"internalError", L"importPrefixPath not found"));
        } else {
            vec<managedPtr<value>> &importPrefixPath = it->second->getVec();
            // 遍历 importPrefixPath 向量中的所有字符串
            for (const auto &prefixPath: importPrefixPath) {
                // 获取字符串对象
                vstr fullPath = prefixPath->isRef() ? prefixPath->getRef().getStr() : prefixPath->getStr();

                path::join(fullPath, path);
                fullPath = path::getRealpath(fullPath);

                try {
                    return importNativeModuleEx(interpreter, fullPath);
                } catch (rex::importError &e) {
                    continue;
                }
            }

            throw importError(L"Cannot open file: file not exist or damaged");
        }
    }

    managedPtr<value> importExternPackage(interpreter *interpreter, const vstr &pkgName) {
        // 获取 importPrefixPath 向量
        if (auto it = interpreter->env->globalCxt->members.find(L"importPrefixPath"); it == interpreter->env->globalCxt->members.end()) {
            throw signalException(interpreter::makeErr(L"internalError", L"importPrefixPath not found"));
        } else {
            vec<managedPtr<value>> &importPrefixPath = it->second->getVec();
            // 遍历 importPrefixPath 向量中的所有字符串
            for (const auto &prefixPath: importPrefixPath) {
                // 获取字符串对象
                vstr fullPath = prefixPath->isRef() ? prefixPath->getRef().getStr() : prefixPath->getStr();

                path::join(fullPath, pkgName);
                fullPath = path::getRealpath(fullPath);

                try {
                    return importExternPackageEx(interpreter, fullPath);
                } catch (rex::importError &e) {
                    continue;
                }
            }
            throw importError(L"Cannot open file: file not exist or damaged");
        }
    }

    managedPtr <value>
    importExternModuleEx(interpreter *interpreter, const vstr &fullPath) {
        if (auto it = interpreter->env->globalCxt->members.find(fullPath); it != interpreter->env->globalCxt->members.end()) {
            return it->second;
        } else {
            std::ifstream f(wstring2string(fullPath), std::ios::in);
            if (f.is_open()) {
                auto moduleCxt = interpreter->moduleCxt;

                interpreter->env->globalCxt->members[fullPath] = moduleCxt;
                moduleCxt->members[L"__path__"] = managePtr(
                        value{fullPath, rex::stringMethods::getMethodsCxt()});
                interpreter->interpreterCxt[L"thread_id"] = rex::managePtr(rex::value{(rex::vint) 0});

                f.seekg(0, std::ios::end);
                long fileLen = f.tellg();
                vbytes buf(fileLen, vbyte{});
                f.seekg(0, std::ios::beg);
                f.read(buf.data(), fileLen);
                std::wstringstream ss(string2wstring(buf));
                buf.clear();

                rex::lexer lexer{ss};
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
                throw importError(L"Cannot open file: file not exist or damaged");
            }
        }
    }

    managedPtr <value> importNativeModuleEx(interpreter *interpreter, const vstr &fullPath) {
        if (auto it = interpreter->env->globalCxt->members.find(fullPath); it != interpreter->env->globalCxt->members.end()) {
            return it->second;
        } else {
            void *handle = dlopen(wstring2string(fullPath).c_str(), RTLD_LAZY);
            if (!handle)
                throw importError(L"Cannot open file: file not exist or damaged");

            auto moduleCxt = interpreter->moduleCxt;

            interpreter->env->globalCxt->members[fullPath] = moduleCxt;
            moduleCxt->members[L"__path__"] = managePtr(
                    value{fullPath, rex::stringMethods::getMethodsCxt()});
            moduleCxt->members[L"__handle__"] = managePtr(value{(unknownPtr) handle});

            using funcPtr = void(const managedPtr<environment> &, const managedPtr<value> &);

            std::function<funcPtr> rexModInit = (funcPtr *) (dlsym(handle, "rexModInit"));

            rexModInit(interpreter->env, moduleCxt);

            return moduleCxt;
        }
    }

    managedPtr<value> importExternPackageEx(interpreter *interpreter, const vstr &pkgDirPath) {
        if (auto it = interpreter->env->globalCxt->members.find(pkgDirPath); it != interpreter->env->globalCxt->members.end()) {
            return it->second;
        } else {
            std::ifstream f(wstring2string(pkgDirPath + L"/packageLoader.rex"), std::ios::in);
            if (f.is_open()) {
                auto moduleCxt = interpreter->moduleCxt;

                interpreter->env->globalCxt->members[pkgDirPath] = moduleCxt;
                moduleCxt->members[L"__path__"] = managePtr(
                        value{pkgDirPath + L"/packageLoader.rex", rex::stringMethods::getMethodsCxt()});
                moduleCxt->members[L"rexPkgRoot"] = managePtr(value{pkgDirPath, rex::stringMethods::getMethodsCxt()});

                f.seekg(0, std::ios::end);
                long fileLen = f.tellg();
                vbytes buf(fileLen, vbyte{});
                f.seekg(0, std::ios::beg);
                f.read(buf.data(), fileLen);
                std::wstringstream ss(string2wstring(buf));
                buf.clear();

                rex::lexer lexer{ss};
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
                throw importError(L"Cannot open file: file not exist or damaged");
            }
        }
    }

    managedPtr <value>
    importEx(interpreter *interpreter, const vstr &modPath) {
        auto oldModCxt = interpreter->moduleCxt;
        auto moduleCxt = rex::managePtr(rex::value{rex::value::cxtObject{}});
        interpreter->moduleCxt = moduleCxt;

        if (auto it = oldModCxt->members.find(L"rexPkgRoot"); it != oldModCxt->members.end()) {
            moduleCxt->members.insert(*it);
        }

        std::filesystem::path p(wstring2string(modPath));
        if (p.has_extension()) {
            if (string2wstring(p.extension()) == L".rex") {
                moduleCxt = rex::importExternModule(interpreter, modPath);
            } else if (string2wstring(p.extension()) == L"." + getDylibSuffix()) {
                moduleCxt = rex::importNativeModule(interpreter, modPath);
            }
        } else {
            moduleCxt = importExternPackage(interpreter, modPath);
        }
        interpreter->moduleCxt = oldModCxt;

        return moduleCxt;
    }

}