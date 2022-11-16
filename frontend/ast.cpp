//
// Created by XIaokang00010 on 2022/11/13.
//

#include "ast.hpp"

namespace rex {
    AST::AST(AST::treeKind K, lexer::token L) : kind(K), leaf(std::move(L)) {

    }

    AST::AST(AST::treeKind K, vec<AST> C) : kind(K), child(std::move(C)) {

    }

    AST::operator bool() {
        return kind != treeKind::notMatch;
    }

    AST::AST() : kind(treeKind::notMatch), child(), leaf() {

    }
} // rex