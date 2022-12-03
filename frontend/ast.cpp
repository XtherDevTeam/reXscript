//
// Created by XIaokang00010 on 2022/11/13.
//

#include "ast.hpp"

namespace rex {
    AST::AST(AST::treeKind k, lexer::token l) : kind(k), leaf(std::move(l)) {

    }

    AST::AST(AST::treeKind k, vec<AST> c) : kind(k), leaf(), child(std::move(c)) {

    }

    AST::operator bool() {
        return kind != treeKind::notMatch;
    }

    AST::AST() : kind(treeKind::notMatch), child(), leaf() {

    }
} // rex