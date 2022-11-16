//
// Created by XIaokang00010 on 2022/11/16.
//

#ifndef REXSCRIPT_LEXERTOASTPASS_HPP
#define REXSCRIPT_LEXERTOASTPASS_HPP

#include "basicPass.hpp"
#include "frontend/lexer.hpp"
#include "frontend/ast.hpp"

namespace rex {

    class lexerToASTPass : public basicPass<lexer, AST> {
    public:
        explicit lexerToASTPass(const rex::lexer &lex);

        AST run() override;
    };

} // rex

#endif //REXSCRIPT_LEXERTOASTPASS_HPP
