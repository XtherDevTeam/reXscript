#include <iostream>
#include "exceptions/errorInAnotherInterpreter.hpp"
#include <interpreter/builtInMethods.hpp>
#include <rex.hpp>
#include <ffi/ffi.hpp>

const char *helpMsg = "Usage: rex [--help] [-m module] [file] args\n"
                      "\n"
                      "Optional arguments: \n"
                      "  -m                execute the specify module \n"
                      "  --generate-ffi\tspecify the FFI config file\n"
                      "  file          \tspecify the file to be executed\n"
                      "  args              the arguments pass to reXscript as `rexArgs`";

void interactiveShell(rex::managedPtr<rex::interpreter> &interpreter) {
    using namespace rex;
    auto moduleCxt = rex::managePtr(rex::value{rex::value::cxtObject{
            {L"__path__", managePtr(value{L"<stdin>", stringMethods::getMethodsCxt()})}
    }});

    interpreter->env->globalCxt->members[L"__local__"] = moduleCxt;
    interpreter->interpreterCxt[L"thread_id"] = rex::managePtr(rex::value{(rex::vint) 0});
    interpreter->stack.push_back({{L"<stdin>", 0, 0}, moduleCxt, {}});
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
            rex::value result = rex::globalMethods::stringify(
                    (void *) interpreter.get(), {interpreter->interpret(ast)}, {});
            std::cout << "output> " << rex::wstring2string(result.getStr()) << std::endl;
        } catch (rex::errorInAnotherInterpreter &e) {
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        } catch (std::exception &e) {
            std::cerr << "error> " << e.what() << std::endl;
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        }
    }
}

void loadFile(rex::managedPtr<rex::interpreter> &interpreter, const rex::vstr &path) {
    try {
        auto moduleCxt = rex::importEx(interpreter.get(), path);
    } catch (rex::errorInAnotherInterpreter &e) {
        std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
    } catch (std::exception &e) {
        std::cerr << "error> " << e.what() << std::endl;
        std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
    }
}

void ffiGenerator(const rex::vstr &path) {
    rex::ffi::ffiDescriptor f;
    f.fromFile(path);
    std::cout << rex::wstring2string(f.generateHeader()) << std::endl;
}

void getArgs(const rex::managedPtr<rex::environment> &env, int argc, const char **argv, const char **pos) {
    for (const char *i = *pos; pos < argv + argc; pos++, i = *pos) {
        env->globalCxt->members[L"rexArgs"]->getVec().push_back(rex::managePtr(
                rex::value{rex::string2wstring(i), rex::stringMethods::getMethodsCxt()}));
    }
}

int main(int argc, const char **argv) {
    rex::rexEnvironmentInstance = rex::getRexEnvironment();

    auto interpreter = rex::managePtr(
            rex::interpreter{rex::rexEnvironmentInstance, rex::managePtr(rex::value{rex::value::cxtObject{}})});
    interpreter->interpreterCxt[L"thread_id"] = rex::managePtr(rex::value{rex::vint{0}});

    if (argc == 1) {
        interactiveShell(interpreter);
        return 0;
    }

    const char **current = argv + 1;
    if (rex::vbytes{*current} == "-m") {
        rex::vstr modPath = rex::string2wstring(*(++current));
        getArgs(rex::rexEnvironmentInstance, argc, argv, ++current);
        rex::importEx(interpreter.get(), modPath);
    } else if (rex::vbytes{*current} == "--generate-ffi") {
        ffiGenerator(rex::string2wstring(*(++current)));
    } else if (rex::vbytes{*current} == "--help") {
        std::cout << helpMsg << std::endl;
    } else {
        rex::vstr file = rex::string2wstring(*current);
        getArgs(rex::rexEnvironmentInstance, argc, argv, ++current);
        loadFile(interpreter, file);
    }
    return 0;
}
