#include <iostream>
#include "exceptions/parserException.hpp"
#include "exceptions/signalException.hpp"
#include "interpreter/builtInMethods.hpp"
#include "interpreter/bytecodeEngine.hpp"
#include "frontend/parser.hpp"
#include <ffi/ffi.hpp>

const char *helpMsg = "Usage: rex [--help] [-m module] [file] args\n"
                      "\n"
                      "Optional arguments: \n"
                      "  -m                execute the specify module \n"
                      "  --generate-ffi\tspecify the FFI config file\n"
                      "  file          \tspecify the file to be executed\n"
                      "  args              the arguments pass to reXscript as `rexArgs`";

void interactiveShell(rex::managedPtr<rex::bytecodeEngine::interpreter> &interpreter) {
    using namespace rex;
    auto moduleCxt = rex::managePtr(rex::value{rex::value::cxtObject{
            {L"__path__", managePtr(value{L"<stdin>", stringMethods::getMethodsCxt()})}
    }});

    auto bm = rex::managePtr(rex::bytecodeEngine::bytecodeModule{});

    interpreter->env->globalCxt->members[L"__local__"] = moduleCxt;
    interpreter->interpreterCxt[L"thread_id"] = rex::managePtr(rex::value{(rex::vint) 0});
    interpreter->callStack.back().pushLocalCxt({});

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
        rex::bytecodeEngine::codeStruct cs;
        rex::bytecodeEngine::codeBuilder cb(*bm, cs);
        cb.buildStmt(ast);
        interpreter->callStack.back().currentCodeStruct = &cs;
        interpreter->callStack.back().programCounter = 0;
        interpreter->interpret();

        try {
            if (!interpreter->evalStack.empty()) {
                rex::value result = rex::globalMethods::stringify(
                        (void *) interpreter.get(), {interpreter->evalStack.back()}, {});
                interpreter->evalStack.pop_back();
                std::cout << "output> " << rex::wstring2string(result.getStr()) << std::endl;
            } else {
                std::cout << "output> no result or optimized by compiler" << std::endl;
            }
        } catch (rex::signalException &e) {
            std::cerr << "exception> " << rex::wstring2string((rex::value) e.get()) << std::endl;
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        } catch (rex::parserException &e) {
            std::cerr << "error> " << e.what() << std::endl;
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        } catch (rex::rexException &e) {
            std::cerr << "error> " << e.what() << std::endl;
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        } catch (std::exception &e) {
            std::cerr << "error> " << e.what() << std::endl;
            std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
        }
    }
}

void loadFile(rex::managedPtr<rex::bytecodeEngine::interpreter> &interpreter, const rex::vstr &path) {
    try {
        auto moduleCxt = rex::bytecodeEngine::require(interpreter.get(), path);
    } catch (rex::signalException &e) {
        std::cerr << "exception> " << rex::wstring2string((rex::value) e.get()) << std::endl;
        std::cerr << rex::wstring2string(interpreter->getBacktrace()) << std::endl;
    } catch (rex::parserException &e) {
        std::cerr << "error> " << e.what() << std::endl;
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

void
getArgs(const rex::managedPtr<rex::bytecodeEngine::environment> &env, int argc, const char **argv, const char **pos) {
    for (const char *i = *pos; pos < argv + argc; pos++, i = *pos) {
        env->globalCxt->members[L"rexArgs"]->getVec().push_back(rex::managePtr(
                rex::value{rex::string2wstring(i), rex::stringMethods::getMethodsCxt()}));
    }
}

int main(int argc, const char **argv) {
    rex::bytecodeEngine::rexEnvironmentInstance = rex::bytecodeEngine::getRexEnvironment();

    auto interpreter = rex::managePtr(
            rex::bytecodeEngine::interpreter{rex::bytecodeEngine::rexEnvironmentInstance, {},
                                             rex::managePtr(rex::value{rex::value::cxtObject{}})});

    if (argc == 1) {
        interactiveShell(interpreter);
        return 0;
    }

    const char **current = argv + 1;
    if (rex::vbytes{*current} == "-m") {
        rex::vstr modPath = rex::string2wstring(*(++current));
        getArgs(rex::bytecodeEngine::rexEnvironmentInstance, argc, argv, ++current);
        rex::bytecodeEngine::requireWithPath(interpreter.get(), modPath);
    } else if (rex::vbytes{*current} == "--generate-ffi") {
        ffiGenerator(rex::string2wstring(*(++current)));
    } else if (rex::vbytes{*current} == "--help") {
        std::cout << helpMsg << std::endl;
    } else {
        rex::vstr file = rex::string2wstring(*current);
        getArgs(rex::bytecodeEngine::rexEnvironmentInstance, argc, argv, ++current);
        loadFile(interpreter, file);
    }
    return 0;
}
