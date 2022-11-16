#include <iostream>
#include <pass/stringToLexerPass.hpp>

int main() {
    std::string buf;
    std::getline(std::cin, buf);
    std::wstring code;
    code = std::move(rex::string2wstring(buf));
    rex::stringToLexerPass pass1(code);
    rex::lexer lexer = pass1.run();
    for (auto tok = lexer.scan(); tok.kind != rex::lexer::token::tokenKind::eof; tok = lexer.scan()) {
        std::cout << rex::wstring2string(tok.value) << std::endl;
    }
    return 0;
}
