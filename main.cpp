#include "frontend/ast.hpp"
#include "frontend/parser.hpp"
#include <iostream>
#include <pass/stringToLexerPass.hpp>
#include <pass/lexerToAstPass.hpp>
#include <interpreter/value.hpp>

int main() {
    std::string buf;
    std::getline(std::cin, buf);
    std::wstring code;
    code = std::move(rex::string2wstring(buf));
    rex::stringToLexerPass pass1(code);
    rex::lexer lexer = pass1.run();
    rex::parser parser{lexer};
    rex::AST result = parser.parseStmts();
    rex::value a(rex::value::vKind::vInt, (rex::value::vValue){(rex::vint)1});
    return 0;
}
