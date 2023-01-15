//
// Created by XIaokang00010 on 2023/1/15.
//

#include <iostream>

#include <frontend/parser.hpp>
#include <interpreter/bytecodeStructs.hpp>
#include <interpreter/bytecodeEngine.hpp>

void dis() {
    rex::bytecodeEngine::bytecodeModule bm;
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
        rex::bytecodeEngine::codeBuilder cb(bm, cs);
        cb.buildStmt(ast);
        std::cout << rex::wstring2string(cs) << std::endl;
    }
}

int main () {
    dis();
    return 0;
}