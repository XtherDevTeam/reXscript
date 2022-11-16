//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_AST_HPP
#define REXSCRIPT_AST_HPP

#include <share/share.hpp>
#include "lexer.hpp"

namespace rex {

    class AST {
    public:
        enum class treeKind {
            notMatch,                   // cannot match
            basicLiterals,              // basic literals like integers, decimals and strings
            identifier,                 // identifier
            subscriptExpression,        // like a[114514], a(114514)
            listLiteral,                // List literals like [111,222,333]
            memberPair,                 // like a: 114514
            objectLiteral,              // like object {"a": 1, "b": 2, c: 3, d: 4}
            arguments,                  // like (a, b, c)
            closureDefinition,          // Closure function definitions like closure () -> () {}
            functionDefinition,         // Function definitions like func () {}
            memberExpression,           // member expression like aaa.bbb.ccc,
                                        // both 2 terms in expression are subscriptExpression
                                        // primary means match a literal or memberExpression
            uniqueExpression,           // like -[primary], --[primary], ++[primary], ![primary]
            multiplicationExpression,   // multiplication expressions like a * b, a / b, a % b
            additionExpression,         // addition expressions like a + b, a - b
            binaryShiftExpression,      // like a << b, a >> b
            logicEqualExpression,       // like a == b, a != b, a >= b, a <= b, a > b, a < b
            binaryExpression,           // like a | b, a & b, a ^ b,
            logicAndExpression,         // like a && b, a || b,
            assignmentExpression,       // like a = b, a += b, a -= b, a *= b, a /= b, a %= b
            blockStmt,                  // like {a; b; c;}
            variableDefOrDeclStmt,      // like var i, j = 0
            variableDefInitExpr,        // like j = 0
            whileStmt,                  // like while (True) {}
            forStmt,                    // like for (a; b; c) {}
            rangeBasedForStmt,          // like forEach (a in b) {}
            ifStmt,                     // like if (True) {}
            ifElseStmt,                 // like if (True) {} else return 0;
            operators,                  // like +, -, *, /
        } kind;

        lexer::token leaf;

        vec<AST> child;

        AST();

        AST(treeKind K, lexer::token L);

        AST(treeKind K, vec<AST> C);

        operator bool();
    };

} // rex

#endif //REXSCRIPT_AST_HPP
