//
// Created by XIaokang00010 on 2022/11/13.
//

#include "parser.hpp"
#include "exceptions/parserException.hpp"

namespace rex {
    parser::parser(lexer &lex) : lex(lex) {
        lex.scan(); // initialize lexer
    }

    AST parser::parseBasicLiterals() {
        // backup
        switch (lex.curToken.kind) {
            // string is not a basic literal because it is an object
            case lexer::token::tokenKind::integer:
            case lexer::token::tokenKind::boolean:
            case lexer::token::tokenKind::decimal:
            case lexer::token::tokenKind::kNull:
            case lexer::token::tokenKind::string: {
                AST res = {AST::treeKind::basicLiterals, lex.curToken};
                lex.scan();
                return res;
            }
            default: {
                return makeNotMatch();
            }
        }
    }

    AST parser::parseIdentifier() {
        if (lex.curToken.kind == lexer::token::tokenKind::identifier) {
            AST res = {AST::treeKind::identifier, lex.curToken};
            lex.scan();
            return res;
        } else {
            return makeNotMatch();
        }
    }

    AST parser::makeNotMatch() {
        return {AST::treeKind::notMatch, lexer::token()};
    }

    AST parser::parseSubscriptExpression() {
        lex.saveState();
        AST base = parseIdentifier();
        // fallback
        if (!base) {
            lex.returnState();
            return makeNotMatch();
        }
        while (lex.curToken.kind == lexer::token::tokenKind::leftBracket or
               lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            switch (lex.curToken.kind) {
                case lexer::token::tokenKind::leftBracket: {
                    lex.scan();
                    AST subscript = parseLvalueExpression();
                    if (subscript) {
                        if (lex.curToken.kind == lexer::token::tokenKind::rightBracket) {
                            lex.scan();
                            base = {AST::treeKind::subscriptExpression, (vec<AST>) {base, subscript}};
                        } else {
                            throw parserException(lex.line, lex.col, L"expected a ']' to close a subscript");
                        }
                    } else {
                        throw parserException(lex.line, lex.col,
                                              L"cannot match a LvalueExpression node for subscriptExpression");
                    }
                    break;
                }
                case lexer::token::tokenKind::leftParentheses: {
                    AST arg = parseArguments();
                    if (arg) {
                        base = {AST::treeKind::invokingExpression, (vec<AST>) {base, arg}};
                    } else {
                        throw parserException(lex.line, lex.col,
                                              L"cannot match a Arguments node for subscriptExpression");
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
        lex.dropState();
        return base;
    }

    AST parser::parseListLiteral() {
        if (lex.curToken.kind != lexer::token::tokenKind::leftBracket) {
            return makeNotMatch();
        }
        lex.scan();
        AST base = {AST::treeKind::listLiteral, (vec<AST>) {}}, current = parseLvalueExpression();
        while (current) {
            base.child.push_back(current);
            if (lex.curToken.kind != lexer::token::tokenKind::comma)
                break;
            lex.scan();
            current = parseLvalueExpression();
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightBracket) {
            throw parserException(lex.line, lex.col, L"expected a ']' to close a list literal");
        }
        lex.scan();
        return base;
    }

    AST parser::parseObjectLiteral() {
//        if (lex.curToken.kind != lexer::token::tokenKind::kObject) {
//            return makeNotMatch();
//        }
//        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftBraces) {
            return makeNotMatch();
        }
        lex.scan();
        AST base = {AST::treeKind::objectLiteral, (vec<AST>) {}};
        while (true) {
            if (lex.curToken.kind != lexer::token::tokenKind::identifier and
                lex.curToken.kind != lexer::token::tokenKind::string) {
                break;
            }
            AST left = {AST::treeKind::identifier, lex.curToken};
            lex.scan();
            if (lex.curToken.kind != lexer::token::tokenKind::colon) {
                throw parserException(lex.line, lex.col, L"expected a `:` after an identifier");
            }
            lex.scan();
            AST right = parseLvalueExpression();
            if (!right) {
                throw parserException(lex.line, lex.col, L"expected a lvalue expression");
            }
            base.child.push_back((AST) {AST::treeKind::memberPair, (vec<AST>) {left, right}});
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            }
            lex.scan();
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightBraces) {
            throw parserException(lex.line, lex.col, L"expected a `}` to close an objectLiteral");
        }
        lex.scan();
        return base;
    }

    AST parser::parseArguments() {
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            return makeNotMatch();
        }
        lex.scan();

        AST base = {AST::treeKind::arguments, (vec<AST>) {}};
        AST current = parseLvalueExpression();
        while (current) {
            base.child.push_back(current);
            if (lex.curToken.kind != lexer::token::tokenKind::comma)
                break;
            lex.scan();
            current = parseLvalueExpression();
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            throw parserException(lex.line, lex.col, L"expected a ')' to close an Arguments node");
        }
        lex.scan();
        return base;
    }

    AST parser::parseClosureDefinition() {
        if (lex.curToken.kind != lexer::token::tokenKind::kClosure)
            return makeNotMatch();
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses)
            throw parserException(lex.line, lex.col, L"expected a '(' to open a Arguments node");
        lex.scan();

        AST passVarsBase = {AST::treeKind::arguments, (vec<AST>) {}};
        AST temp = parseIdentifier();
        while (temp) {
            passVarsBase.child.push_back(temp);
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            }
            lex.scan();
            temp = parseIdentifier();
        }

        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses)
            throw parserException(lex.line, lex.col, L"expected a ')' to close a Arguments node");
        lex.scan();

        if (lex.curToken.kind != lexer::token::tokenKind::toSign)
            throw parserException(lex.line, lex.col, L"expected '->'");
        lex.scan();

        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses)
            throw parserException(lex.line, lex.col, L"expected a '(' to open a Arguments node");
        lex.scan();

        AST argumentsNode = {AST::treeKind::arguments, (vec<AST>) {}};
        temp = parseIdentifier();
        while (temp) {
            argumentsNode.child.push_back(temp);
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            }
            lex.scan();
            temp = parseIdentifier();
        }

        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses)
            throw parserException(lex.line, lex.col, L"expected a ')' to close a Arguments node");
        lex.scan();

        if (!argumentsNode)
            throw parserException(lex.line, lex.col, L"expected an Arguments node");

        AST blockNode = parseBlockStmt();
        if (!blockNode)
            throw parserException(lex.line, lex.col, L"expected a code block");

        return {AST::treeKind::lambdaDefinition, (vec<AST>) {passVarsBase, argumentsNode, blockNode}};
    }

    AST parser::parseFunctionDefinition() {
        if (lex.curToken.kind != lexer::token::tokenKind::kFunc)
            return makeNotMatch();
        lex.scan();

        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses)
            throw parserException(lex.line, lex.col, L"expected a '(' to open a Arguments node");
        lex.scan();

        AST argumentsNode = {AST::treeKind::arguments, (vec<AST>) {}};
        AST temp = parseIdentifier();
        while (temp) {
            argumentsNode.child.push_back(temp);
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            }
            lex.scan();
            temp = parseIdentifier();
        }

        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses)
            throw parserException(lex.line, lex.col, L"expected a ')' to close a Arguments node");
        lex.scan();

        AST blockNode = parseBlockStmt();
        if (!blockNode)
            throw parserException(lex.line, lex.col, L"expected a code block");

        return {AST::treeKind::functionDefinition, (vec<AST>) {argumentsNode, blockNode}};
    }

    AST parser::parseMemberExpression() {
        AST vLhs = parsePrimary();
        if (!vLhs)
            return makeNotMatch();
        if (lex.curToken.kind != lexer::token::tokenKind::dot)
            return vLhs;
        lex.scan();
        AST vRhs = parseSubscriptExpression();
        if (!vRhs)
            throw parserException(lex.line, lex.col, L"expected a right-hand-side node after `.`");
        while (lex.curToken.kind == lexer::token::tokenKind::dot) {
            vLhs = {AST::treeKind::memberExpression, {vLhs, vRhs}};
            lex.scan();
            vRhs = parseSubscriptExpression();
            if (!vRhs) {
                throw parserException(lex.line, lex.col, L"expected a right-hand-side node after `.`");
            }
        }
        if (vRhs)
            vLhs = {AST::treeKind::memberExpression, {vLhs, vRhs}};
        return vLhs;
    }

    AST parser::parsePrimary() {
        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
            AST expr = parseLvalueExpression();
            if (!expr)
                throw parserException(lex.line, lex.col, L"expected a LvalueExpression node after `(`");
            if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses)
                throw parserException(lex.line, lex.col, L"expected `)`");
            lex.scan();
            return expr;
        }

        AST node = parseBasicLiterals();
        if (node)
            return node;

        node = parseListLiteral();
        if (node)
            return node;

        node = parseObjectLiteral();
        if (node)
            return node;

        node = parseClosureDefinition();
        if (node)
            return node;

        node = parseFunctionDefinition();
        if (node)
            return node;

        node = parseSubscriptExpression();
        if (node)
            return node;

        return makeNotMatch();
    }

    AST parser::parseUniqueExpression() {
        lex.saveState();

        if (lex.curToken.kind == lexer::token::tokenKind::incrementSign or
            lex.curToken.kind == lexer::token::tokenKind::decrementSign or
            lex.curToken.kind == lexer::token::tokenKind::minus or
            lex.curToken.kind == lexer::token::tokenKind::binaryAnd or
            lex.curToken.kind == lexer::token::tokenKind::asterisk) {
            AST vOperator = {AST::treeKind::operators, lex.curToken};
            lex.scan();
            AST vItem = parseMemberExpression();

            if (!vItem) {
                lex.returnState();
                return makeNotMatch();
            }

            lex.dropState();
            return {AST::treeKind::uniqueExpression, (vec<AST>) {vOperator, vItem}};
        } else {
            AST vItem = parseMemberExpression();
            if (!vItem) {
                lex.returnState();
                return makeNotMatch();
            }
            lex.dropState();
            return vItem;
        }
    }

    AST parser::parseMultiplicationExpression() {
        AST vLhs = parseUniqueExpression();
        if (!vLhs)
            return makeNotMatch();
        if (lex.curToken.kind != lexer::token::tokenKind::asterisk and
            lex.curToken.kind != lexer::token::tokenKind::slash and
            lex.curToken.kind != lexer::token::tokenKind::percentSign)
            return vLhs;
        AST vOp = (AST) {AST::treeKind::operators, lex.curToken};
        lex.scan();
        AST vRhs = parseUniqueExpression();
        if (!vRhs)
            throw parserException(lex.line, lex.col, L"expected a right-hand-side node after operators");
        while (lex.curToken.kind == lexer::token::tokenKind::asterisk or
               lex.curToken.kind == lexer::token::tokenKind::slash or
               lex.curToken.kind == lexer::token::tokenKind::percentSign) {
            vOp = (AST) {AST::treeKind::operators, lex.curToken};
            vLhs = {AST::treeKind::multiplicationExpression, {vLhs, vOp, vRhs}};
            lex.scan();
            vRhs = parseUniqueExpression();
            if (!vRhs) {
                throw parserException(lex.line, lex.col, L"expected a right-hand-side node after operators");
            }
        }
        if (vRhs)
            vLhs = {AST::treeKind::multiplicationExpression, {vLhs, vOp, vRhs}};
        return vLhs;
    }

    AST parser::parseAdditionExpression() {
        AST vLhs = parseMultiplicationExpression();
        if (!vLhs)
            return makeNotMatch();
        if (lex.curToken.kind != lexer::token::tokenKind::plus and lex.curToken.kind != lexer::token::tokenKind::minus)
            return vLhs;
        AST vOp = {AST::treeKind::operators, lex.curToken};
        lex.scan();
        AST vRhs = parseMultiplicationExpression();
        while (lex.curToken.kind == lexer::token::tokenKind::plus or
               lex.curToken.kind == lexer::token::tokenKind::minus) {
            vLhs = {AST::treeKind::additionExpression, {vLhs, vOp, vRhs}};
            vOp = {AST::treeKind::operators, lex.curToken};
            lex.scan();
            vRhs = parseMultiplicationExpression();
            if (!vRhs) {
                throw parserException(lex.line, lex.col, L"expected a right-hand-side node after operators");
            }
        }
        if (vRhs)
            vLhs = {AST::treeKind::additionExpression, {vLhs, vOp, vRhs}};
        return vLhs;
    }

    AST parser::parseBinaryShiftExpression() {
        AST vLhs = parseAdditionExpression();
        if (!vLhs)
            return makeNotMatch();
        if (lex.curToken.kind != lexer::token::tokenKind::binaryShiftLeft and
            lex.curToken.kind != lexer::token::tokenKind::binaryShiftRight)
            return vLhs;
        AST vOp = {AST::treeKind::operators, lex.curToken};
        lex.scan();
        AST vRhs = parseAdditionExpression();
        while (lex.curToken.kind == lexer::token::tokenKind::binaryShiftLeft or
               lex.curToken.kind == lexer::token::tokenKind::binaryShiftRight) {
            vLhs = {AST::treeKind::binaryShiftExpression, {vLhs, vOp, vRhs}};
            vOp = {AST::treeKind::operators, lex.curToken};
            lex.scan();
            vRhs = parseAdditionExpression();
            if (!vRhs) {
                throw parserException(lex.line, lex.col, L"expected a right-hand-side node after operators");
            }
        }
        if (vRhs)
            vLhs = {AST::treeKind::binaryShiftExpression, {vLhs, vOp, vRhs}};
        return vLhs;
    }

    AST parser::parseLogicEqualExpression() {
        AST vLhs = parseBinaryShiftExpression();
        if (!vLhs)
            return makeNotMatch();
        if (lex.curToken.kind != lexer::token::tokenKind::equal and
            lex.curToken.kind != lexer::token::tokenKind::notEqual and
            lex.curToken.kind != lexer::token::tokenKind::greaterEqual and
            lex.curToken.kind != lexer::token::tokenKind::lessEqual and
            lex.curToken.kind != lexer::token::tokenKind::greaterThan and
            lex.curToken.kind != lexer::token::tokenKind::lessThan)
            return vLhs;
        AST vOp = {AST::treeKind::operators, lex.curToken};
        lex.scan();
        AST vRhs = parseBinaryShiftExpression();
        while (lex.curToken.kind == lexer::token::tokenKind::equal or
               lex.curToken.kind == lexer::token::tokenKind::notEqual or
               lex.curToken.kind == lexer::token::tokenKind::greaterEqual or
               lex.curToken.kind == lexer::token::tokenKind::lessEqual or
               lex.curToken.kind == lexer::token::tokenKind::greaterThan or
               lex.curToken.kind == lexer::token::tokenKind::lessThan) {
            vLhs = {AST::treeKind::logicEqualExpression, {vLhs, vOp, vRhs}};
            vOp = {AST::treeKind::operators, lex.curToken};
            lex.scan();
            vRhs = parseBinaryShiftExpression();
            if (!vRhs) {
                throw parserException(lex.line, lex.col, L"expected a right-hand-side node after operators");
            }
        }
        if (vRhs)
            vLhs = {AST::treeKind::logicEqualExpression, {vLhs, vOp, vRhs}};
        return vLhs;
    }

    AST parser::parseBinaryExpression() {
        AST vLhs = parseLogicEqualExpression();
        if (!vLhs)
            return makeNotMatch();
        if (lex.curToken.kind != lexer::token::tokenKind::binaryAnd and
            lex.curToken.kind != lexer::token::tokenKind::binaryOr and
            lex.curToken.kind != lexer::token::tokenKind::binaryXor)
            return vLhs;
        AST vOp = {AST::treeKind::operators, lex.curToken};
        lex.scan();
        AST vRhs = parseLogicEqualExpression();
        while (lex.curToken.kind == lexer::token::tokenKind::binaryAnd or
               lex.curToken.kind == lexer::token::tokenKind::binaryOr or
               lex.curToken.kind == lexer::token::tokenKind::binaryXor) {
            vLhs = {AST::treeKind::binaryExpression, {vLhs, vOp, vRhs}};
            vOp = {AST::treeKind::operators, lex.curToken};
            lex.scan();
            vRhs = parseLogicEqualExpression();
            if (!vRhs) {
                throw parserException(lex.line, lex.col, L"expected a right-hand-side node after operators");
            }
        }
        if (vRhs)
            vLhs = {AST::treeKind::binaryExpression, {vLhs, vOp, vRhs}};
        return vLhs;
    }

    AST parser::parseLogicAndExpression() {
        AST vLhs = parseBinaryExpression();
        if (!vLhs)
            return makeNotMatch();
        if (lex.curToken.kind != lexer::token::tokenKind::logicAnd and
            lex.curToken.kind != lexer::token::tokenKind::logicOr)
            return vLhs;
        AST vOp = {AST::treeKind::operators, lex.curToken};
        lex.scan();
        AST vRhs = parseBinaryExpression();
        while (lex.curToken.kind == lexer::token::tokenKind::logicAnd or
               lex.curToken.kind == lexer::token::tokenKind::logicOr) {
            vLhs = {AST::treeKind::logicAndExpression, {vLhs, vOp, vRhs}};
            vOp = {AST::treeKind::operators, lex.curToken};
            lex.scan();
            vRhs = parseBinaryExpression();
            if (!vRhs) {
                throw parserException(lex.line, lex.col, L"expected a right-hand-side node after operators");
            }
        }
        if (vRhs)
            vLhs = {AST::treeKind::logicAndExpression, {vLhs, vOp, vRhs}};
        return vLhs;
    }

    AST parser::parseLvalueExpression() {
        return parseLogicAndExpression();
    }

    AST parser::parseAssignmentExpression() {
        lex.saveState();
        AST vLhs = parseMemberExpression();
        if (!vLhs) {
            lex.dropState();
            return makeNotMatch();
        }
        if (lex.curToken.kind != lexer::token::tokenKind::additionAssignment and
            lex.curToken.kind != lexer::token::tokenKind::subtractionAssignment and
            lex.curToken.kind != lexer::token::tokenKind::multiplicationAssignment and
            lex.curToken.kind != lexer::token::tokenKind::divisionAssignment and
            lex.curToken.kind != lexer::token::tokenKind::reminderAssignment and
            lex.curToken.kind != lexer::token::tokenKind::assignSign) {
            lex.returnState();
            return makeNotMatch();
        }
        AST vOp = {AST::treeKind::operators, lex.curToken};
        lex.scan();
        AST vRhs = parseLvalueExpression();
        if (!vRhs) {
            throw parserException(lex.line, lex.col, L"expected a right-hand-side node after operators");
        }

        lex.dropState();
        return {AST::treeKind::assignmentExpression, {vLhs, vOp, vRhs}};
    }

    AST parser::parseBlockStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::leftBraces) {
            return makeNotMatch();
        }
        lex.scan();
        AST base = {AST::treeKind::blockStmt, (vec<AST>) {}};
        AST stmt = parseStmts();
        while (stmt) {
            switch (stmt.kind) {
                case AST::treeKind::ifStmt:
                case AST::treeKind::ifElseStmt:
                case AST::treeKind::whileStmt:
                case AST::treeKind::forStmt:
                case AST::treeKind::rangeBasedForStmt:
                case AST::treeKind::blockStmt:
                    break;
                default: {
                    if (lex.curToken.kind != lexer::token::tokenKind::semicolon) {
                        throw parserException(lex.line, lex.col, L"expected `;` after statements");
                    }
                    lex.scan();
                    break;
                }
            }
            base.child.push_back(stmt);
            stmt = parseStmts();
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightBraces) {
            return makeNotMatch();
        }
        lex.scan();
        return base;
    }

    AST parser::parseLetStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kLet) {
            return makeNotMatch();
        }
        lex.scan();
        AST base{AST::treeKind::letStmt, (vec<AST>) {}};
        AST lhs, rhs;
        while (true) {
            lhs = parseIdentifier();
            if (!lhs) {
                throw parserException(lex.line, lex.col, L"expected at least one assignment");
            }
            if (lex.curToken.kind == lexer::token::tokenKind::assignSign) {
                lex.scan();
                rhs = parseLvalueExpression();
                if (!rhs) {
                    throw parserException(lex.line, lex.col, L"expected LvalueExpression after assignSign");
                }
            } else {
                throw parserException(lex.line, lex.col, L"expected `=`");
            }

            base.child.push_back((AST) {AST::treeKind::letAssignmentPair, (vec<AST>) {lhs, rhs}});

            if (lex.curToken.kind != lexer::token::tokenKind::comma)
                break;
            lex.scan();
        }

        return base;
    }

    AST parser::parseWhileStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kWhile) {
            return makeNotMatch();
        }
        lex.scan();

        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses)
            throw parserException(lex.line, lex.col, L"expected `(` after `while`");
        lex.scan();

        AST condition = parseLvalueExpression();
        if (!condition)
            throw parserException(lex.line, lex.col, L"expected LvalueExpression after `(`");
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses)
            throw parserException(lex.line, lex.col, L"expected `)` after condition");
        lex.scan();

        AST stmt = parseStmts();
        if (!stmt)
            throw parserException(lex.line, lex.col, L"expected statements after `)`");

        return {AST::treeKind::whileStmt, (vec<AST>) {condition, stmt}};
    }

    AST parser::parseForStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kFor) {
            return makeNotMatch();
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses)
            throw parserException(lex.line, lex.col, L"expected `(` after `for`");
        lex.scan();

        AST initializationStmt = parseStmts();
        if (!initializationStmt)
            throw parserException(lex.line, lex.col, L"expected statements after `(`");

        if (lex.curToken.kind != lexer::token::tokenKind::semicolon)
            throw parserException(lex.line, lex.col, L"expected `;` after statement");
        lex.scan();

        AST condition = parseLvalueExpression();
        if (!condition)
            throw parserException(lex.line, lex.col, L"expected a condition after `;`");

        if (lex.curToken.kind != lexer::token::tokenKind::semicolon)
            throw parserException(lex.line, lex.col, L"expected `;` after the condition");
        lex.scan();

        AST updateStmt = parseStmts();
        if (!updateStmt)
            throw parserException(lex.line, lex.col, L"expected a statement after the `;`");

        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses)
            throw parserException(lex.line, lex.col, L"expected `)` after the statement");
        lex.scan();

        AST stmt = parseStmts();

        if (!stmt)
            throw parserException(lex.line, lex.col, L"expected a statement after the `)`");

        return {AST::treeKind::forStmt, {initializationStmt, condition, updateStmt, stmt}};
    }

    AST parser::parseRangeBasedForStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kForEach) {
            return makeNotMatch();
        }
        lex.scan();

        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses)
            throw parserException(lex.line, lex.col, L"expected `(` after `forEach`");
        lex.scan();

        AST identifier = parseIdentifier();
        if (!identifier)
            throw parserException(lex.line, lex.col, L"expected an identifier after `(`");

        if (lex.curToken.kind != lexer::token::tokenKind::kIn)
            throw parserException(lex.line, lex.col, L"expected `in` after the identifier");
        lex.scan();

        AST expression = parseLvalueExpression();
        if (!identifier)
            throw parserException(lex.line, lex.col, L"expected an expression after `in`");

        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses)
            throw parserException(lex.line, lex.col, L"expected `)` after the expression");
        lex.scan();

        AST stmt = parseStmts();
        if (!stmt)
            throw parserException(lex.line, lex.col, L"expected statements after `)`");

        return {AST::treeKind::rangeBasedForStmt, {identifier, expression, stmt}};
    }

    AST parser::parseIfStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kIf) {
            return makeNotMatch();
        }
        lex.scan();

        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses)
            throw parserException(lex.line, lex.col, L"expected `(` after `while`");
        AST condition = parseLvalueExpression();
        if (!condition)
            throw parserException(lex.line, lex.col, L"expected LvalueExpression after `(`");
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses)
            throw parserException(lex.line, lex.col, L"expected `)` after condition");

        AST stmt = parseStmts();
        if (!stmt)
            throw parserException(lex.line, lex.col, L"expected statements after `)`");

        if (lex.curToken.kind != lexer::token::tokenKind::kElse)
            return {AST::treeKind::ifStmt, (vec<AST>) {condition, stmt}};

        AST elseStmt = parseStmts();
        if (!stmt)
            throw parserException(lex.line, lex.col, L"expected statements after `else`");

        return {AST::treeKind::ifElseStmt, {condition, stmt, elseStmt}};
    }

    AST parser::parseReturnStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kReturn) {
            return makeNotMatch();
        }
        lex.scan();

        AST result = parseLvalueExpression();
        if (!result)
            throw parserException(lex.line, lex.col, L"expected LvalueExpressions after `return`");

        return {AST::treeKind::returnStmt, (vec<AST>) {result}};
    }

    AST parser::parseContinueStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kContinue) {
            return makeNotMatch();
        }
        lex.scan();

        return {AST::treeKind::continueStmt, (lexer::token) {}};
    }

    AST parser::parseBreakStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kBreak) {
            return makeNotMatch();
        }
        lex.scan();

        return {AST::treeKind::breakStmt, (lexer::token) {}};
    }

    AST parser::parseStmts() {
        AST result;

        result = parseTryCatchStmt();
        if (result)
            return result;

        result = parseThrowStmt();
        if (result)
            return result;

        result = parseReturnStmt();
        if (result)
            return result;

        result = parseContinueStmt();
        if (result)
            return result;

        result = parseBreakStmt();
        if (result)
            return result;

        result = parseLetStmt();
        if (result)
            return result;

        result = parseWhileStmt();
        if (result)
            return result;

        result = parseForStmt();
        if (result)
            return result;

        result = parseRangeBasedForStmt();
        if (result)
            return result;

        result = parseIfStmt();
        if (result)
            return result;

        result = parseBlockStmt();
        if (result)
            return result;

        result = parseAssignmentExpression();
        if (result)
            return result;

        result = parseLvalueExpression();
        return result;
    }

    AST parser::parseFile() {
        AST base = {AST::treeKind::blockStmt, (vec<AST>) {}};
        AST stmt = parseStmts();
        while (stmt) {
            switch (stmt.kind) {
                case AST::treeKind::ifStmt:
                case AST::treeKind::ifElseStmt:
                case AST::treeKind::whileStmt:
                case AST::treeKind::forStmt:
                case AST::treeKind::rangeBasedForStmt:
                case AST::treeKind::blockStmt:
                    break;
                default: {
                    if (lex.curToken.kind != lexer::token::tokenKind::semicolon) {
                        throw parserException(lex.line, lex.col, L"expected `;` after statements");
                    }
                    lex.scan();
                    break;
                }
            }
            base.child.push_back(stmt);
            stmt = parseStmts();
        }

        return base;
    }

    AST parser::parseTryCatchStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kTry)
            return {};
        lex.scan();
        AST tryBlock = parseBlockStmt();
        if (!tryBlock)
            throw parserException(lex.line, lex.col, L"expected blockStmt after `try`");
        if (lex.curToken.kind != lexer::token::tokenKind::kCatch)
            throw parserException(lex.line, lex.col, L"expected `catch` after blockStmt");
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::kAs)
            throw parserException(lex.line, lex.col, L"expected `as` after `catch`");
        lex.scan();
        AST catchId = parseIdentifier();
        if (!catchId)
            throw parserException(lex.line, lex.col, L"expected an identifier after `as`");
        AST catchBlock = parseBlockStmt();
        if (!catchBlock)
            throw parserException(lex.line, lex.col, L"expected blockStmt after identifier");
        return {AST::treeKind::tryCatchStmt, {tryBlock, catchId, catchBlock}};
    }

    AST parser::parseThrowStmt() {
        if (lex.curToken.kind != lexer::token::tokenKind::kThrow)
            return {};
        lex.scan();
        AST expr = parseLvalueExpression();
        if (!expr)
            throw parserException(lex.line, lex.col, L"expected LvalueExpression after `throw`");
        return {AST::treeKind::throwStmt, {expr}};
    }
} // rex