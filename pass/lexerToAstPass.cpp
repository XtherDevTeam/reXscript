//
// Created by XIaokang00010 on 2022/11/16.
//

#include "lexerToAstPass.hpp"
#include <frontend/parser.hpp>

namespace rex {
    lexerToASTPass::lexerToASTPass(lexer &lex) : basicPass(lex) {

    }

    AST lexerToASTPass::run() {
        parser p{basicPass::before};

        return p.parseFile();
    }
} // rex