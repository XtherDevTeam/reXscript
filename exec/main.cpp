#include <iostream>
#include "exceptions/parserException.hpp"
#include "exceptions/signalException.hpp"
#include <rex.hpp>
#include <ffi/ffi.hpp>

const char *helpMsg = "Usage: rex [--help] [-m module] [file] args\n"
                      "\n"
                      "Optional arguments: \n"
                      "  -m                execute the specify module \n"
                      "  --generate-ffi\tspecify the FFI config file\n"
                      "  file          \tspecify the file to be executed\n"
                      "  args              the arguments pass to reXscript as `rexArgs`";

void interactiveShell(rex::managedPtr<rex::environment> &env) {
    auto moduleCxt = rex::managePtr(rex::value{rex::value::cxtObject{}});
    env->globalCxt->members[L"__local__"] = moduleCxt;
    auto interpreter = rex::managePtr(rex::interpreter{env, moduleCxt});
    interpreter->interpreterCxt[L"thread_id"] = rex::managePtr(rex::value{(rex::vint) 0});
    interpreter->stack.emplace_back();
    interpreter->stack.back().pushLocalCxt({});
    while (std::cin) {
        std::string buf;
        std::cout << "input> ";
        std::getline(std::cin, buf);
        if (buf.empty())
            continue;
        std::wstring code;
        code = std::move(rex::string2wstring(buf));
        std::wistringstream ss(code);
        code.clear();
        rex::lexer lexer{ss};
        rex::parser parser{lexer};
        rex::AST ast = parser.parseStmts();
        try {
            rex::value result = interpreter->interpret(ast);
            std::cout << "output> " << rex::wstring2string(result) << std::endl;
        } catch (rex::signalException &e) {
            std::cerr << "exception> " << rex::wstring2string((rex::value) e.get()) << std::endl;
        } catch (rex::parserException &e) {
            std::cerr << "error> " << e.what() << std::endl;
        } catch (std::exception &e) {
            std::cerr << "error> " << e.what() << std::endl;
        }
    }
}

void loadFile(rex::managedPtr<rex::environment> &env, const rex::vstr &path) {
    auto moduleCxt = rex::importExternModule(env, path, {});
}

void ffiGenerator(const rex::vstr &path) {
    rex::ffi::ffiDescriptor f;
    f.fromFile(path);
    std::cout << rex::wstring2string(f.generateHeader()) << std::endl;
}

void getArgs(const rex::managedPtr<rex::environment> &env, int argc, const char **argv, const char** pos) {
    for (const char *i = *pos; pos < argv +  argc; pos++, i = *pos) {
        env->globalCxt->members[L"rexArgs"]->getVec().push_back(rex::managePtr(
                rex::value{rex::string2wstring(i), rex::stringMethods::getMethodsCxt()}));
    }
}

int main(int argc, const char **argv) {
    auto env = rex::getRexEnvironment();

    if (argc == 1) {
        interactiveShell(env);
        return 0;
    }

    try {
        const char **current = argv + 1;
        if (rex::vbytes{*current} == "-m") {
            rex::vstr modPath = rex::string2wstring(*(++current));
            getArgs(env, argc, argv, ++current);
            rex::importEx(env, modPath, {});
        } else if (rex::vbytes{*current} == "--generate-ffi") {
            ffiGenerator(rex::string2wstring(*(++current)));
        } else if (rex::vbytes{*current} == "--help") {
            std::cout << helpMsg << std::endl;
        } else {
            rex::vstr file = rex::string2wstring(*current);
            getArgs(env, argc, argv, ++current);
            loadFile(env, file);
        }
        return 0;
    } catch (rex::signalException &e) {
        std::cerr << "exception> " << rex::wstring2string((rex::value) e.get()) << std::endl;
    } catch (rex::rexException &e) {
        std::cerr << "error> " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << "error> " << e.what() << std::endl;
    }
    return 0;
}
