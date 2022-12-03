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

        value makeErr(const vstr &errName, const vstr &errMsg);

        value invokeFunc(value *val, const vec<value> &args, value *passThisPtr = nullptr);

        value *getRvalue(const AST & target);

        value interpret(const AST & target);
    };

    using nativeFuncPtr = std::function<value(interpreter *, vec<value> &)>;
}

#endif