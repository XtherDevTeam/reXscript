//
// Created by XIaokang00010 on 2023/1/14.
//

#ifndef REXSCRIPT_BYTECODEENGINE_HPP
#define REXSCRIPT_BYTECODEENGINE_HPP

#include <cstdint>
#include "share/share.hpp"
#include "frontend/ast.hpp"
#include "value.hpp"
#include "bytecodeStructs.hpp"

namespace rex::bytecodeEngine {
    struct environment {
        struct runtimeSourceFileMsg {
            vstr file;
            vsize line;
            vsize col;

            operator vstr();
        };

        static runtimeSourceFileMsg dumpRuntimeSourceFileMsg(const value::funcObject &func);

        static runtimeSourceFileMsg dumpRuntimeSourceFileMsg(const value::lambdaObject &lambda);

        /**
         * @brief Represents a stack frame for a function call
         */
        struct stackFrame {
            /**
             * @brief The context of the module the function belongs to
             */
            managedPtr<value> moduleCxt;
            /**
             * @brief A list of local variable contexts for the function
             * A new local variable context is pushed into this list whenever a new code block is entered
             */
            vec<value::cxtObject> localCxt;

            runtimeSourceFileMsg sourceMsg;

            /**
             * @brief Pushes a new local variable context into the local variable context list
             * @param cxt The local variable context to be pushed
             */
            void pushLocalCxt(const value::cxtObject &cxt);

            /**
             * @brief Pops the top local variable context from the local variable context list
             */
            void popLocalCxt();

            /**
             * @brief Gets the index of the current local variable context in the local variable context list
             * @return The index of the current local variable context
             */
            vsize getCurLocalCxtIdx();

            /**
             * @brief Rolls back to the specified local variable context
             * @param idx The index of the local variable context to roll back to
             */
            void backToLocalCxt(vsize idx);

            stackFrame(runtimeSourceFileMsg msg);

            stackFrame(runtimeSourceFileMsg msg, managedPtr<value> &moduleCxt, const vec<value::cxtObject> &localCxt);

            operator vstr();
        };

        /**
         * @brief Represents a thread in the environment
         */
        class thread {
            /**
             * @brief The std::thread object
             */
            managedPtr<std::thread> th;
            /**
             * @brief The result value of the thread
             */
            managedPtr<value> result;
        public:
            /**
             * @brief Constructs a new thread object
             */
            thread();

            /**
             * @brief Sets the std::thread object
             * @param v The std::thread object to set
             */
            void setTh(const managedPtr<std::thread> &v);

            /**
             * @brief Gets the std::thread object
             * @return The std::thread object
             */
            const managedPtr<std::thread> &getTh();

            /**
             * @brief Sets the result value of the thread
             * @param v The result value to set
             */
            void setResult(const managedPtr<value> &v);

            /**
             * @brief Gets the result value of the thread
             * @return The result value of the thread
             */
            const managedPtr<value> &getResult();
        };


        /**
         * @brief The global variable context
         */
        managedPtr<value> globalCxt;

        /**
         * @brief A pool of threads in the environment
         */
        map<vint, thread> threadPool;

        /**
         * @brief A counter for generating unique thread IDs
         */
        vint threadIdCounter{1};

        /**
         * @brief global executable bytecode structs
         */
        vec<managedPtr<codeStruct>> codeStructs;

        uint64_t putCodeStruct(const managedPtr<codeStruct> &v);
    };

    extern managedPtr<environment> rexEnvironmentInstance;

    class codeBuilder {
        bytecodeModule mod;
        codeStruct &currentBlock;
    public:
        codeBuilder(bytecodeModule mod, codeStruct &currentBlock);

        uint64_t getNextCur();

        void buildBasicLiterals(const AST &target);

        void buildIdentifier(const AST &target);

        void buildSubscriptExpr(const AST &target);

        void buildInvokeExpr(const AST &target);

        void buildListLiteral(const AST &target);

        void buildObjectLiteral(const AST &target);

        void buildLambdaDef(const AST &target);

        void buildFuncDef(const AST &target);

        void buildMemberExpr(const AST &target);

        void buildUniqueExpr(const AST &target);

        void buildMulExpr(const AST &target);

        void buildAddExpr(const AST &target);

        void buildBinShiftExpr(const AST &target);

        void buildLogicEqExpr(const AST &target);

        void buildBinExpr(const AST &target);

        void buildLogicAndExpr(const AST &target);

        void buildAssignmentExpr(const AST &target);

        void buildBlockStmt(const AST &target);

        void buildLetStmt(const AST &target);

        void buildWhileStmt(const AST &target);

        void buildForStmt(const AST &target);

        void buildForEachStmt(const AST &target);

        void buildIfStmt(const AST &target);

        void buildIfElseStmt(const AST &target);

        void buildReturnStmt(const AST &target);

        void buildContinueStmt(const AST &target);

        void buildBreakStmt(const AST &target);

        void buildTryCatchStmt(const AST &target);

        void buildThrowStmt(const AST &target);

        void buildFuncDefStmt(const AST &target);

        void buildStmt(const AST &target);

        void buildExpr(const AST &target);
    };
}

#endif //REXSCRIPT_BYTECODEENGINE_HPP
