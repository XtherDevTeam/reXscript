//
// Created by XIaokang00010 on 2022/11/13.
//

#include "stringToLexerPass.hpp"

namespace rex {
    stringToLexerPass::stringToLexerPass(const vstr &before) : basicPass(before), ss(before) {

    }

    lexer stringToLexerPass::run() {
        return lexer(ss);
    }
} // rex