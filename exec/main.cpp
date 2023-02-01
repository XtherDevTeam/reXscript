#include <iostream>
#include "exceptions/errorInAnotherInterpreter.hpp"
#include "exceptions/importError.hpp"
#include "exceptions/parserException.hpp"
#include "exceptions/signalException.hpp"
#include <interpreter/builtInMethods.hpp>
#include <rex.hpp>
#include <ffi/ffi.hpp>

#if defined(_WIN32)
#include <windows.h>
BOOL WINAPI rexAtExitWin32Handler(DWORD type) {
    rex::atExitHandler();
    std::exit(0);
}
bool reXscriptIsTheBestProgrammingLanguageInTheWorld = []() {
    SetConsoleCtrlHandler(rexAtExitWin32Handler, TRUE);
    return true;
}();
#else
void rexAtExitUnixHandler(int fn) {
    rex::atExitHandler();
    std::exit(0);
}
bool reXscriptIsTheBestProgrammingLanguageInTheWorld = []() {
    signal(SIGINT, rexAtExitUnixHandler);
    signal(SIGTERM, rexAtExitUnixHandler);
    signal(SIGKILL, rexAtExitUnixHandler);
    return true;
}();
#endif

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
        try {
            rex::lexer lexer{ss};
            rex::parser parser{lexer};
            rex::AST ast = parser.parseStmts();

            rex::value result = rex::globalMethods::stringify(
                    (void *) interpreter.get(), {interpreter->interpret(ast)}, {});
            std::cout << "output> " << rex::wstring2string(result.getStr()) << std::endl;
        } catch (rex::importError &e) {
            std::cerr << "importError> " << e.what() << std::endl;
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        } catch (rex::signalException &e) {
            std::cerr << "exception> " << rex::wstring2string((rex::value) e.get()) << std::endl;
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        } catch (rex::parserException &e) {
            std::cerr << "error> " << e.what() << std::endl;
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        } catch (rex::errorInAnotherInterpreter &e) {
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        } catch (rex::rexException &e) {
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
    } catch (rex::importError &e) {
        std::cerr << "importError> " << e.what() << std::endl;
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

    rex::rexInterpreterInstance = rex::getRexInterpreter();

    if (argc == 1) {
        interactiveShell(rex::rexInterpreterInstance);
        return 0;
    }

    const char **current = argv + 1;
    if (rex::vbytes{*current} == "-m") {
        rex::vstr modPath = rex::string2wstring(*(++current));
        getArgs(rex::rexEnvironmentInstance, argc, argv, ++current);
        try {
            rex::importEx(rex::rexInterpreterInstance.get(), modPath);
        } catch (rex::errorInAnotherInterpreter &e) {
            std::cerr << rex::wstring2string(rex::rexInterpreterInstance->getBacktrace()) << std::endl;
        } catch (rex::importError &e) {
            std::cerr << "importError> " << e.what() << std::endl;
            std::cerr << rex::wstring2string(rex::rexInterpreterInstance->getBacktrace()) << std::endl;
        } catch (std::exception &e) {
            std::cerr << "error> " << e.what() << std::endl;
            std::cerr << rex::wstring2string(rex::rexInterpreterInstance->getBacktrace()) << std::endl;
        }
    } else if (rex::vbytes{*current} == "--generate-ffi") {
        ffiGenerator(rex::string2wstring(*(++current)));
    } else if (rex::vbytes{*current} == "--help") {
        std::cout << helpMsg << std::endl;
    } else {
        rex::vstr file = rex::string2wstring(*current);
        getArgs(rex::rexEnvironmentInstance, argc, argv, ++current);
        loadFile(rex::rexInterpreterInstance, file);
    }
    rex::atExitHandler();
    std::exit(0);
}
