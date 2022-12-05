#include "frontend/ast.hpp"
#include "frontend/parser.hpp"
#include <iostream>
#include "pass/stringToLexerPass.hpp"
#include "interpreter/value.hpp"
#include "interpreter/builtInMethods.hpp"
#include "interpreter/interpreter.hpp"

void interactiveShell(rex::managedPtr<rex::interpreter> &interpreter) {
    interpreter->stack.emplace_back();
    interpreter->stack.back().pushLocalCxt({});
    while (std::cin) {
        std::string buf;
        std::cout << "input> ";
        std::getline(std::cin, buf);
        std::wstring code;
        code = std::move(rex::string2wstring(buf));
        rex::stringToLexerPass pass1(code);
        rex::lexer lexer = pass1.run();
        rex::parser parser{lexer};
        rex::AST ast = parser.parseStmts();
        rex::value result = interpreter->interpret(ast);
        std::cout << "output> " << rex::wstring2string(result) << std::endl;
    }
}

void lexTest() {
    std::wstring s = L"wdnmdnmsl.wqnmgb()";
    rex::stringToLexerPass pass1(s);
    rex::lexer lexer = pass1.run();
    rex::parser parser{lexer};
    rex::AST ast = parser.parseStmts();
    std::cout << (char *) s.c_str() << std::endl;
}

void fuckIstringstream() {
    std::wstring s = L"a\\n";
    std::wistringstream ss{s};
    rex::vchar ch{};
    while (ss) {
        if(!ss.get(ch)) break;
        printf("%c", ch);
    }
}

int main() {
    rex::managedPtr<rex::environment> env = rex::managePtr(rex::environment{});
    rex::managedPtr<rex::value> moduleCxt = rex::managePtr(rex::value{});
    env->globalCxt = rex::managePtr(rex::value{rex::globalMethods::getMethodsCxt()});
    env->globalCxt->members[L"__local__"] = moduleCxt;
    rex::managedPtr<rex::interpreter> interpreter = rex::managePtr(rex::interpreter{env, moduleCxt});
    interactiveShell(interpreter);
//    lexTest();
//fuckIstringstream();
    return 0;
}
