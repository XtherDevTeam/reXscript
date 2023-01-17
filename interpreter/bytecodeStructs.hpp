//
// Created by XIaokang00010 on 2023/1/15.
//

#ifndef REXSCRIPT_BYTECODESTRUCTS_HPP
#define REXSCRIPT_BYTECODESTRUCTS_HPP

#include "share/share.hpp"

namespace rex::bytecodeEngine {
    struct bytecodeStruct {
        enum class opCode : int64_t {
            unknown,
            pushLocalCxt,
            popLocalCxt,
            jump,
            jumpIfTrue,
            jumpIfFalse,
            pushExceptionHandler,
            popExceptionHandler,
            invoke,
            ret,
            find,
            findAttr,
            index,
            invokeMethod,
            opIncrement,
            opDecrement,
            opNegate,
            opAdd,
            opSub,
            opMul,
            opDiv,
            opMod,
            opBinaryShiftLeft,
            opBinaryShiftRight,
            opEqual,
            opNotEqual,
            opGreaterEqual,
            opLessEqual,
            opGreaterThan,
            opLessThan,
            opBinaryOr,
            opBinaryAnd,
            opBinaryXor,
            opLogicAnd,
            opLogicOr,
            assign,
            addAssign,
            subAssign,
            mulAssign,
            divAssign,
            modAssign,
            intConst,
            deciConst,
            boolConst,
            nullConst,
            stringNew,
            arrayNew,
            objectNew,
            funcNew,
            lambdaNew,
            putIndex,
            duplicate,
            deepCopy,
            createOrAssign,
            fakeOpBreak,
            fakeOpContinue,
            opThrow,
            popTop,
        } opcode{};

        union opArgs {
            vint intv;
            vdeci deciv;
            vbool boolv;
            uint64_t indexv;

            opArgs();

            opArgs(vint v);

            opArgs(vdeci v);

            opArgs(vbool v);

            opArgs(uint64_t v);
        } opargs;

        operator vstr();
    };

    struct runtimeSourceFileMsg {
        vstr file;
        vsize line;
        vsize col;

        operator vstr();
    };

    struct codeStruct {
        runtimeSourceFileMsg msg;

        vec<bytecodeStruct> code;   // code array
        vec<vstr> names;            // names from local and extern
        vec<vstr> stringConsts;     // string constants

        uint64_t putStringConst(const vstr &v);

        uint64_t putNames(const vstr &v);

        operator vstr();
    };

    struct bytecodeModule {
        uint64_t entryBlock;

        vec<managedPtr<codeStruct>> codeStructs;

        uint64_t putCodeStruct(const codeStruct &v);
    };
}

#endif //REXSCRIPT_BYTECODESTRUCTS_HPP
