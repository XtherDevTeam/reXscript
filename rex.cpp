//
// Created by XIaokang00010 on 2022/12/5.
//

#include <fstream>
#include <share/dlfcn.hpp>
#include "rex.hpp"
#include "exceptions/signalException.hpp"
#include "exceptions/endOfFileException.hpp"
#include "exceptions/importError.hpp"
#include "exceptions/parserException.hpp"
#include "exceptions/errorInAnotherInterpreter.hpp"

namespace rex {

    rex::managedPtr<rex::environment> getRexEnvironment() {
        auto env = rex::managePtr(rex::environment{managePtr(value{globalMethods::getMethodsCxt()})});
        auto cstr = std::getenv("rexModulesPath");
        auto str = rex::string2wstring(cstr == nullptr ? "" : cstr);
        if (!str.empty()) {
            split(str, vstr{L";"}, [&](const vstr &i, vsize) {
                env->globalCxt->members[L"importPrefixPath"]->getVec().push_back(
                        managePtr(value{i, rex::stringMethods::getMethodsCxt()}));
            });
        }
        env->globalCxt->members[L"rexArgs"] = managePtr(value{value::vecObject{}, vecMethods::getMethodsCxt()});
        env->globalCxt->members[L"rexAtExit"] = managePtr(value{value::vecObject{}, vecMethods::getMethodsCxt()});
        return env;
    }

    managedPtr<value>
    importExternModule(interpreter *interpreter, const vstr &path) {
        // 获取 importPrefixPath 向量
        if (auto it = interpreter->env->globalCxt->members.find(L"importPrefixPath"); it ==
                                                                                      interpreter->env->globalCxt->members.end()) {
            throw signalException(interpreter::makeErr(L"internalError", L"importPrefixPath not found"));
        } else {
            vec<managedPtr<value>> &importPrefixPath = it->second->getVec();
            // 遍历 importPrefixPath 向量中的所有字符串
            for (const auto &prefixPath: importPrefixPath) {
                // 获取字符串对象
                vstr fullPath = eleGetRef(*prefixPath).getStr();

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

    managedPtr<value>
    importNativeModule(interpreter *interpreter, const vstr &path) {
        // Thanks for AI's help
        // 获取 importPrefixPath 向量
        if (auto it = interpreter->env->globalCxt->members.find(L"importPrefixPath"); it ==
                                                                                      interpreter->env->globalCxt->members.end()) {
            throw signalException(interpreter::makeErr(L"internalError", L"importPrefixPath not found"));
        } else {
            vec<managedPtr<value>> &importPrefixPath = it->second->getVec();
            // 遍历 importPrefixPath 向量中的所有字符串
            for (const auto &prefixPath: importPrefixPath) {
                // 获取字符串对象
                vstr fullPath = eleGetRef(*prefixPath).getStr();

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
        if (auto it = interpreter->env->globalCxt->members.find(L"importPrefixPath"); it ==
                                                                                      interpreter->env->globalCxt->members.end()) {
            throw signalException(interpreter::makeErr(L"internalError", L"importPrefixPath not found"));
        } else {
            vec<managedPtr<value>> &importPrefixPath = it->second->getVec();
            // 遍历 importPrefixPath 向量中的所有字符串
            for (const auto &prefixPath: importPrefixPath) {
                // 获取字符串对象
                vstr fullPath = eleGetRef(*prefixPath).getStr();

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

    managedPtr<value>
    importExternModuleEx(interpreter *interpreter, const vstr &fullPath) {
        if (auto it = interpreter->env->globalCxt->members.find(fullPath); it !=
                                                                           interpreter->env->globalCxt->members.end()) {
            return it->second;
        } else {
            auto moduleCxt = interpreter->moduleCxt;

            interpreter->env->globalCxt->members[fullPath] = moduleCxt;
            moduleCxt->members[L"__path__"] = managePtr(
                    value{fullPath, rex::stringMethods::getMethodsCxt()});

            try {
                rex::AST ast = getFileAST(fullPath);
                for (auto &i: ast.child) {
                    interpreter->interpret(i);
                }
                if (auto vit = moduleCxt->members.find(L"rexModInit"); vit != moduleCxt->members.end()) {
                    interpreter->invokeFunc(vit->second, {}, {});
                }
            } catch (rex::importError &e) {
                throw;
            } catch (rex::signalException &e) {
                std::cerr << "exception> " << rex::wstring2string((rex::value) e.get()) << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            } catch (rex::parserException &e) {
                std::cerr << "error> file " << wstring2string(fullPath) << e.what() << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            } catch (rex::errorInAnotherInterpreter &e) {
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw;
            } catch (rex::rexException &e) {
                std::cerr << "error> " << e.what() << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            }
            return moduleCxt;
        }
    }

    managedPtr<value> importNativeModuleEx(interpreter *interpreter, const vstr &fullPath) {
        if (auto it = interpreter->env->globalCxt->members.find(fullPath); it !=
                                                                           interpreter->env->globalCxt->members.end()) {
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

            try {
                rexModInit(interpreter->env, moduleCxt);
            } catch (rex::signalException &e) {
                std::cerr << "exception> " << rex::wstring2string((rex::value) e.get()) << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            } catch (rex::parserException &e) {
                std::cerr << "error> file " << wstring2string(fullPath) << e.what() << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            } catch (rex::errorInAnotherInterpreter &e) {
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw;
            } catch (rex::rexException &e) {
                std::cerr << "error> " << e.what() << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            }

            return moduleCxt;
        }
    }

    managedPtr<value> importExternPackageEx(interpreter *interpreter, const vstr &pkgDirPath) {
        if (auto it = interpreter->env->globalCxt->members.find(pkgDirPath); it !=
                                                                             interpreter->env->globalCxt->members.end()) {
            return it->second;
        } else {
            auto moduleCxt = interpreter->moduleCxt;

            interpreter->env->globalCxt->members[pkgDirPath] = moduleCxt;
            moduleCxt->members[L"__path__"] = managePtr(
                    value{pkgDirPath + L"/packageLoader.rex", rex::stringMethods::getMethodsCxt()});
            moduleCxt->members[L"rexPkgRoot"] = managePtr(value{pkgDirPath, rex::stringMethods::getMethodsCxt()});

            try {
                rex::AST ast = getFileAST(pkgDirPath + L"/packageLoader.rex");
                for (auto &i: ast.child) {
                    interpreter->interpret(i);
                }
                if (auto vit = moduleCxt->members.find(L"rexModInit"); vit != moduleCxt->members.end()) {
                    interpreter->invokeFunc(vit->second, {}, {});
                }
            } catch (rex::importError &e) {
                throw;
            } catch (rex::signalException &e) {
                std::cerr << "exception> " << rex::wstring2string((rex::value) e.get()) << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            } catch (rex::parserException &e) {
                std::cerr << "error> file " << wstring2string(pkgDirPath + L"/packageLoader.rex") << e.what() << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            } catch (rex::errorInAnotherInterpreter &e) {
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw;
            } catch (rex::rexException &e) {
                std::cerr << "error> " << e.what() << std::endl;
                std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
                throw errorInAnotherInterpreter();
            }
            return moduleCxt;
        }
    }

    managedPtr<value>
    importEx(interpreter *interpreter, const vstr &modPath) {
        auto moduleCxt = rex::managePtr(rex::value{rex::value::cxtObject{}});
        rex::interpreter newIn(interpreter->env, moduleCxt);
        newIn.interpreterCxt = interpreter->interpreterCxt;

        if (auto it = interpreter->moduleCxt->members.find(L"rexPkgRoot"); it !=
                                                                           interpreter->moduleCxt->members.end()) {
            moduleCxt->members.insert(*it);
        }

        std::filesystem::path p(wstring2string(modPath));
        if (p.has_extension()) {
            if (string2wstring(p.extension()) == L".rex") {
                moduleCxt = rex::importExternModule(&newIn, modPath);
            } else if (string2wstring(p.extension()) == L"." + getDylibSuffix()) {
                moduleCxt = rex::importNativeModule(&newIn, modPath);
            } else {
                moduleCxt = rex::importExternPackage(&newIn, modPath);
            }
        } else {
            moduleCxt = rex::importExternPackage(&newIn, modPath);
        }

        return moduleCxt;
    }

    AST getFileAST(const vstr &path) {
        std::ifstream f(wstring2string(path), std::ios::in);
        if (f.is_open()) {
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
            return ast;
        } else {
            throw importError(L"Cannot open file: file not exist or damaged");
        }
    }

    rex::managedPtr<rex::interpreter> getRexInterpreter() {
        rex::managedPtr<rex::interpreter> result = rex::managePtr(
                rex::interpreter{rex::rexEnvironmentInstance, rex::managePtr(rex::value{rex::value::cxtObject{}})});
        result->interpreterCxt[L"thread_id"] = rex::managePtr(rex::value{rex::vint{0}});

        return result;
    }

    void atExitHandler() {
        try {
            auto &v = rex::rexEnvironmentInstance->globalCxt->members[L"rexAtExit"]->getVec();
            for (auto it = v.rbegin(); it != v.rend(); it++) {
                rexInterpreterInstance->invokeFunc(*it, {}, {});
            }
        } catch (rex::signalException &e) {
            std::cerr << "Uncaught exception at exit handler> " << rex::wstring2string((rex::value) e.get())
                      << std::endl;
            std::cerr << wstring2string(rexInterpreterInstance->getBacktrace()) << std::endl;
        } catch (rex::rexException &e) {
            std::cerr << "Uncaught exception at exit handler> " << e.what() << std::endl;
            std::cerr << wstring2string(rexInterpreterInstance->getBacktrace()) << std::endl;
        } catch (std::exception &e) {
            std::cerr << "Uncaught exception at exit handler> " << e.what() << std::endl;
            std::cerr << wstring2string(rexInterpreterInstance->getBacktrace()) << std::endl;
        }

        if (rexInterpreterInstance)
            rexInterpreterInstance.reset();
        if (rexEnvironmentInstance)
            rexEnvironmentInstance.reset();
    }

    void initializeAtExitHandler() {
        signal(SIGINT, atExitWrapper);
        signal(SIGTERM, atExitWrapper);
    }

    void atExitWrapper(int sig) {
        atExitHandler();
        std::exit(0);
    }

    void initialize() {
        rex::rexEnvironmentInstance = rex::getRexEnvironment();
        rex::rexInterpreterInstance = rex::getRexInterpreter();
        initializeAtExitHandler();
    }
}