//
// Created by XIaokang00010 on 2023/1/15.
//

#include <iostream>

#include <frontend/parser.hpp>
#include <interpreter/bytecodeStructs.hpp>
#include <interpreter/bytecodeEngine.hpp>
#include <interpreter/builtInMethods.hpp>

void dis() {
    auto bm = rex::managePtr(rex::bytecodeEngine::bytecodeModule{});
    auto mod = rex::managePtr(rex::value{});
    mod->members[L"__code__"] = rex::managePtr(rex::value{bm});
    rex::bytecodeEngine::rexEnvironmentInstance->globalCxt = rex::managePtr(
            rex::value{rex::globalMethods::getMethodsCxt()});
    rex::bytecodeEngine::interpreter interpreter{rex::bytecodeEngine::rexEnvironmentInstance, {}, mod};
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

        interpreter.callStack.back().currentCodeStruct = &cs;
        interpreter.callStack.back().programCounter = 0;
        std::cout << rex::wstring2string(cs) << std::endl;

        interpreter.interpret();
    }
}

int main() {
    dis();
    return 0;
}