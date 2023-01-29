//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_PARSER_HPP
#define REXSCRIPT_PARSER_HPP

#include <frontend/lexer.hpp>
#include <frontend/ast.hpp>

namespace rex {

    class parser {
        lexer &lex;
    public:
        parser(lexer &lex);

        static AST makeNotMatch();

        AST parseBasicLiterals();

        AST parseIdentifier();

        AST parseSubscriptExpression();

        AST parseListLiteral();

        AST parseObjectLiteral();

        AST parseArguments();

        AST parseClosureDefinition();

        AST parseFunctionDefinition();

        AST parseMemberExpression();

        AST parsePrimary();

        AST parseUniqueExpression();

        AST parseMultiplicationExpression();

        AST parseAdditionExpression();

        AST parseBinaryShiftExpression();

        AST parseLogicEqualExpression();

        AST parseBinaryExpression();

        AST parseLogicAndExpression();

        AST parseLvalueExpression(); // a set of all lvalue expressions

        AST parseAssignmentExpression();

        AST parseBlockStmt();

        AST parseLetStmt();

        AST parseWhileStmt();

        AST parseForStmt();

        AST parseRangeBasedForStmt();

        AST parseIfStmt();

        AST parseReturnStmt();

        AST parseContinueStmt();

        AST parseBreakStmt();

        AST parseTryCatchStmt();

        AST parseThrowStmt();

        AST parseFunctionDefStmt();

        AST parseWithStmt();

        AST parseStmts(); // a set of all statements

        AST parseFile(); // parse a whole file which is from passes
    };

} // rex

#endif //REXSCRIPT_PARSER_HPP
