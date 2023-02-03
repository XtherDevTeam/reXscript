//
// Created by XIaokang00010 on 2022/12/3.
//

#ifndef REXSCRIPT_INTERPRETER_HPP
#define REXSCRIPT_INTERPRETER_HPP

#include "share/share.hpp"
#include "value.hpp"
#include <thread>

namespace rex {

    struct environment {
        struct runtimeSourceFileMsg {
            vstr file;
            vsize line;
            vsize col;

            operator vstr();
        };

        static runtimeSourceFileMsg dumpRuntimeSourceFileMsg(const managedPtr<value> &moduleCxt, AST target);

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

    };

    extern managedPtr<environment> rexEnvironmentInstance;

    /**
     * @brief The interpreter class for reXscript
     */
    class interpreter {
    public:
        /**
         * @brief A stack for storing the stack frames for function calls
         */
        vec<environment::stackFrame> stack;

        /**
         * @brief The context of the module currently being interpreted
         */
        managedPtr<value> moduleCxt;

        /**
         * @brief A pointer to the environment in which the interpreter is running
         */
        managedPtr<environment> env;

        /**
         * @brief The interpreter context for storing global variables for the interpreter
         */
        value::cxtObject interpreterCxt;

        /**
         * @brief Get the current index of the stack
         *
         * @return The index of the current stack frame
         */
        vsize getCurStackIdx();

        /**
         * @brief Set the current index of the stack
         *
         * @param stkIdx The index to set the current stack frame to
         */
        void backToStackIdx(vsize stkIdx);

        /**
         * @brief Construct a new interpreter object
         *
         * @param env The environment in which the interpreter will be running
         * @param moduleCxt The context of the module currently being interpreted
         */
        interpreter(const managedPtr<environment> &env, const managedPtr<value> &moduleCxt);

        // @brief Create a new error value with the specified error name and message.
        static value makeErr(const vstr &errName, const vstr &errMsg);

        static value makeIt(const managedPtr<value> &v, bool isEnd);

        // @brief Invoke a function with the given arguments and `this` pointer.
        value invokeFunc(managedPtr<value> func, const vec<value> &args, const managedPtr<value> &passThisPtr);

        vstr getBacktrace();

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

        value interpretLvalueExpressions(const AST &target);

        value interpretAssignments(const AST &target);

        // 整合解释器和符号查找器
        // 对于Rvalue -> 返回ref形式
        // 对于Lvalue -> 返回value形式
        value interpret(const AST &target);
    };

    extern managedPtr<interpreter> rexInterpreterInstance;

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

#define eleGetRef(ele) ((ele).isRef() ? (ele).getRef() : (ele))

#define eleRefObj(ele) ((ele).isRef() ? (ele).refObj : rex::managePtr((ele)))
} // rex

#endif //REXSCRIPT_INTERPRETER_HPP
