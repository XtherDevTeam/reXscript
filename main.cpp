#include <frontend/ast.hpp>
#include <frontend/parser.hpp>
#include <iostream>
#include <pass/stringToLexerPass.hpp>
#include "interpreter/value.hpp"
#include "interpreter/interpreter.hpp"

int main() {
    std::string buf;
    std::getline(std::cin, buf);
    std::wstring code;
    code = std::move(rex::string2wstring(buf));
    rex::stringToLexerPass pass1(code);
    rex::lexer lexer = pass1.run();
    rex::parser parser{lexer};
    rex::AST ast = parser.parseStmts();
    rex::managedPtr<rex::environment> env = rex::managePtr(rex::environment{});
    rex::managedPtr<rex::interpreter> interpreter = rex::managePtr(rex::interpreter{env, {}});
    rex::value result = interpreter->interpret(ast);
    return 0;
}
