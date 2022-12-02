//
// Created by XIaokang00010 on 2022/11/30.
//

#ifndef REXSCRIPT_INTERPRETER_HPP
#define REXSCRIPT_INTERPRETER_HPP

#include "frontend/ast.hpp"
#include "interpreter/value.hpp"
#include <functional>
#include <interpreter/heap.hpp>

namespace rex {
    struct environment {
        struct stackFrame {
            heap::vItem *globalContext;
            vec<map<vstr, value>> localContext;
            
            stackFrame();

            stackFrame(heap::vItem *globCxt);

            stackFrame(heap::vItem *globCxt, const map<vstr, value> &initLocalCxt);

            void pushLocalContext(const map<vstr, value> &initLocalCxt);

            void popLocalContext();
        };
        heap envHeap;
        map<vstr, heap::vItem *> loadedModules; // save the runtime modules in heap
        
        vec<stackFrame> envStack;
    };

    class interpreter {
        environment *env;
    public:
        interpreter();

        interpreter(environment *env);

        value parseBasicLiterals(const AST &target);

        value parseIdentifier(const AST &target);

        value parseSubscriptExpression(const AST &target);

        value parseListLiteral(const AST &target);

        value parseObjectLiteral(const AST &target);

        value parseArguments(const AST &target);

        value parseClosureDefinition(const AST &target);

        value parseFunctionDefinition(const AST &target);

        value parseMemberExpression(const AST &target);

        value parsePrimary(const AST &target);

        value parseUniqueExpression(const AST &target);

        value parseMultiplicationExpression(const AST &target);

        value parseAdditionExpression(const AST &target);

        value parseBinaryShiftExpression(const AST &target);

        value parseLogicEqualExpression(const AST &target);

        value parseBinaryExpression(const AST &target);

        value parseLogicAndExpression(const AST &target);

        value parseLvalueExpression(const AST &target); // a set of all lvalue expressions

        value parseAssignmentExpression(const AST &target);

        value parseBlockStmt(const AST &target);

        value parseVariableDefOrDeclStmt(const AST &target);

        value parseWhileStmt(const AST &target);

        value parseForStmt(const AST &target);

        value parseRangeBasedForStmt(const AST &target);

        value parseIfStmt(const AST &target);

        value parseReturnStmt(const AST &target);

        value parseContinueStmt(const AST &target);

        value parseBreakStmt(const AST &target);

        value parseStmts(const AST &target); // a set of all statements
    };

    using nativeFuncPtr = std::function<value(interpreter *, vec<value> &)>;
}

#endif