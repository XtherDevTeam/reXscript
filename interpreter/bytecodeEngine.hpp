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

            codeStruct *currentCodeStruct{};

            uint64_t programCounter{};

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

            stackFrame();

            stackFrame(const managedPtr<value> &moduleCxt, const vec<value::cxtObject> &localCxt, codeStruct *code);

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

    class interpreter {
    public:
        struct state {
            uint64_t frame;
            uint64_t localCxt;
            uint64_t program;
            uint64_t evalStack;
        };

        managedPtr<environment> env;
        vec<environment::stackFrame> callStack;
        managedPtr<value> interpreterCxt;
        managedPtr<value> moduleCxt;
        vec<value> evalStack;
        vec<state> exceptionHandlers;

        /**
         * @brief ABI interface for native functions
         * @warning THIS FUNCTION WILL BREAK THE SIGNLE-INTERPRET-FUNCTION RULE, ONLY FOR FORIGN INTERFACES
         */
        value invokeFunc(const managedPtr<value> &func, const vec<value> &args, const managedPtr<value> &passThisPtr);

        /**
         * @brief throw error in native functions
         * @note interpreter will make a wrapper for the exceptions in native functions, and make sure it could be catch by internal exception handler
         * @warning DO NOT TRY TO CATCH THE EXCEPTION BY YOURSELF, IT WILL BREAK THE STACK FRAMES.
         * @param err the error to throw
         */
        static void throwErr(const value &err);

        static value makeErr(const vstr &errName, const vstr &errMsg);

        static value makeIt(const managedPtr<value> &left, bool isEnd);

        void execute(bytecodeStruct &op);

        void interpret();

        interpreter() = default;

        interpreter(const managedPtr<environment> &env, const managedPtr<value> &interpreterCxt,
                    const managedPtr<value> &moduleCxt);

        vstr getBacktrace();

        void restoreState(const state &s);

        bytecodeModule getBytecodeModule();

        /**
         * @brief prepare for the function call stack frame
         * @param func the function body
         */
        void prepareForFuncInvoke(const managedPtr<value> &func, uint64_t argc);

        void prepareForMethodInvoke(const managedPtr<value> &func, uint64_t argc);

        void prepareForLambdaInvoke(const managedPtr<value> &func, uint64_t argc);

        void prepareForLambdaMethodInvoke(const managedPtr<value> &func, uint64_t argc);

        value invokeNativeFn(const managedPtr<value> &func, uint64_t argc);

        value invokeNativeFn(const managedPtr<value> &func, const vec<value> &args,
                             const managedPtr<value> &passThisPtr);

        value invokeNativeMethod(const managedPtr<value> &func, uint64_t argc);

        value opAdd(value &a, value &b);

        value opSub(value &a, value &b);

        value opMul(value &a, value &b);

        value opDiv(value &a, value &b);

        value opMod(value &a, value &b);

        value opBinaryShiftLeft(value &a, value &b);

        value opBinaryShiftRight(value &a, value &b);

        value opEqual(value &a, value &b);

        value opNotEqual(value &a, value &b);

        value opGreaterEqual(value &a, value &b);

        value opLessEqual(value &a, value &b);

        value opGreaterThan(value &a, value &b);

        value opLessThan(value &a, value &b);

        value opBinaryOr(value &a, value &b);

        value opBinaryAnd(value &a, value &b);

        value opBinaryXor(value &a, value &b);

        value opLogicAnd(value &a, value &b);

        value opLogicOr(value &a, value &b);

        value opIncrement(const managedPtr<value> &ptr);

        value opDecrement(const managedPtr<value> &ptr);

        value opNegate(value &a);

        value opAssign(const managedPtr<value> &ptr, value &a);

        value opAddAssign(const managedPtr<value> &ptr, value &a);

        value opSubAssign(const managedPtr<value> &ptr, value &a);

        value opMulAssign(const managedPtr<value> &ptr, value &a);

        value opDivAssign(const managedPtr<value> &ptr, value &a);

        value opModAssign(const managedPtr<value> &ptr, value &a);

        value opIndex(const managedPtr<value> &ptr, value &a);
    };

    /**
     * @brief compile file
     * @param filepath
     * @return the bytecode module
     */
    bytecodeModule compile(const vstr &filepath);

    /**
     * @brief Wrapper function for starting a new thread. This function is passed as a parameter to std::thread's constructor.
     *
     * @param env The environment in which the new thread will be executed.
     * @param tid The ID of the new thread.
     * @param cxt The context object in which the new thread will be executed.
     * @param func The function object to be called in the new thread.
     * @param args The arguments to be passed to the function object.
     * @param passThisPtr The object to be passed as the this pointer to the function object.
     */
    void
    rexThreadWrapper(managedPtr<environment> env, vint tid, managedPtr<value> cxt, managedPtr<value> func,
                     vec<value> args,
                     managedPtr<value> passThisPtr = nullptr);

    /**
     * @brief Spawns a new thread and returns its ID.
     *
     * @param env The environment in which the new thread will be executed.
     * @param cxt The context object in which the new thread will be executed.
     * @param func The function object to be called in the new thread.
     * @param args The arguments to be passed to the function object.
     * @param passThisPtr The object to be passed as the this pointer to the function object.
     *
     * @return The ID of the new thread.
     */
    vint spawnThread(const managedPtr<environment> &env, const managedPtr<value> &cxt, const managedPtr<value> &func,
                     const vec<value> &args, const managedPtr<value> &passThisPtr = nullptr);

    /**
     * @brief Waits for the thread with the specified ID to finish execution and returns the result.
     *
     * @param env The environment in which the thread is being executed.
     * @param id The ID of the thread to wait for.
     *
     * @return The result of the thread.
     */
    value waitForThread(const managedPtr<environment> &env, vint id);

#define eleGetRef(ele) (ele.isRef() ? ele.getRef() : ele)

#define eleRefObj(ele) (ele.isRef() ? ele.refObj : rex::managePtr(ele))

#define nextOp (callStack.back().programCounter++)
}

#endif //REXSCRIPT_BYTECODEENGINE_HPP
