//
// Created by XIaokang00010 on 2023/1/15.
//

#include "bytecodeStructs.hpp"

namespace rex::bytecodeEngine {
    bytecodeStruct::opArgs::opArgs() : indexv() {

    }

    bytecodeStruct::opArgs::opArgs(vint v) : intv(v) {

    }

    bytecodeStruct::opArgs::opArgs(vdeci v) : deciv(v) {

    }

    bytecodeStruct::opArgs::opArgs(vbool v) : boolv(v) {

    }

    bytecodeStruct::opArgs::opArgs(uint64_t v) : indexv(v) {

    }


    uint64_t codeStruct::putStringConst(const vstr &v) {
        if (auto it = std::find(stringConsts.begin(), stringConsts.end(), v); it != stringConsts.end()) {
            return (uint64_t) (it - stringConsts.begin());
        } else {
            stringConsts.push_back(v);
            return stringConsts.size() - 1;
        }
    }

    uint64_t codeStruct::putNames(const vstr &v) {
        if (auto it = std::find(names.begin(), names.end(), v); it != names.end()) {
            return (uint64_t) (it - names.begin());
        } else {
            names.push_back(v);
            return names.size() - 1;
        }
    }

    codeStruct::operator vstr() {
        uint64_t index;
        std::wstringstream ss;
        ss << L"Bytecode:" << std::endl;
        for (auto &i: code) {
            ss << "\t" << (vstr)
            i << std::endl;
        }
        ss << L"\nNames:" << std::endl;
        index = 0;
        for (auto &i: names) {
            ss << "\t[" << index << "] = " << i << std::endl;
            index++;
        }
        ss << L"\nStrings:" << std::endl;
        index = 0;
        for (auto &i: stringConsts) {
            ss << "\t[" << index << "] = " << std::quoted(i) << std::endl;
            index++;
        }
        ss << std::endl;
        return ss.str();
    }

    uint64_t bytecodeModule::putCodeStruct(const codeStruct &v) {
        codeStructs.push_back(managePtr(v));
        return codeStructs.size() - 1;
    }

    bytecodeStruct::operator vstr() {
        std::wstringstream ss;
        switch (opcode) {
            case opCode::unknown:
                ss << "unknown";
                break;
            case opCode::pushLocalCxt:
                ss << "pushLocalCxt";
                break;
            case opCode::popLocalCxt:
                ss << "popLocalCxt";
                break;
            case opCode::jump:
                ss << "jump " << opargs.intv;
                break;
            case opCode::jumpIfTrue:
                ss << "jumpIfTrue " << opargs.intv;
                break;
            case opCode::jumpIfFalse:
                ss << "jumpIfFalse " << opargs.intv;
                break;
            case opCode::pushExceptionHandler:
                ss << "pushExceptionHandler " << opargs.intv;
                break;
            case opCode::popExceptionHandler:
                ss << "popExceptionHandler";
                break;
            case opCode::invoke:
                ss << "invoke " << opargs.indexv;
                break;
            case opCode::ret:
                ss << "ret";
                break;
            case opCode::find:
                ss << "find " << opargs.indexv;
                break;
            case opCode::findAttr:
                ss << "findAttr " << opargs.indexv;
                break;
            case opCode::index:
                ss << "index";
                break;
            case opCode::invokeMethod:
                ss << "invokeMethod " << opargs.indexv;
                break;
            case opCode::opIncrement:
                ss << "opIncrement";
                break;
            case opCode::opDecrement:
                ss << "opDecrement";
                break;
            case opCode::opNegate:
                ss << "opNegate";
                break;
            case opCode::opAdd:
                ss << "opAdd";
                break;
            case opCode::opSub:
                ss << "opSub";
                break;
            case opCode::opMul:
                ss << "opMul";
                break;
            case opCode::opDiv:
                ss << "opDiv";
                break;
            case opCode::opMod:
                ss << "opMod";
                break;
            case opCode::opBinaryShiftLeft:
                ss << "opBinaryShiftLeft";
                break;
            case opCode::opBinaryShiftRight:
                ss << "opBinaryShiftRight";
                break;
            case opCode::opEqual:
                ss << "opEqual";
                break;
            case opCode::opNotEqual:
                ss << "opNotEqual";
                break;
            case opCode::opGreaterEqual:
                ss << "opGreaterEqual";
                break;
            case opCode::opLessEqual:
                ss << "opLessEqual";
                break;
            case opCode::opGreaterThan:
                ss << "opGreaterThan";
                break;
            case opCode::opLessThan:
                ss << "opLessThan";
                break;
            case opCode::opBinaryOr:
                ss << "opBinaryOr";
                break;
            case opCode::opBinaryAnd:
                ss << "opBinaryAnd";
                break;
            case opCode::opBinaryXor:
                ss << "opBinaryXor";
                break;
            case opCode::opLogicAnd:
                ss << "opLogicAnd";
                break;
            case opCode::opLogicOr:
                ss << "opLogicOr";
                break;
            case opCode::assign:
                ss << "assign";
                break;
            case opCode::addAssign:
                ss << "addAssign";
                break;
            case opCode::subAssign:
                ss << "subAssign";
                break;
            case opCode::mulAssign:
                ss << "mulAssign";
                break;
            case opCode::divAssign:
                ss << "divAssign";
                break;
            case opCode::modAssign:
                ss << "modAssign";
                break;
            case opCode::intConst:
                ss << "intConst " << opargs.intv;
                break;
            case opCode::deciConst:
                ss << "deciConst " << opargs.deciv;
                break;
            case opCode::boolConst:
                ss << "boolConst " << opargs.boolv;
                break;
            case opCode::nullConst:
                ss << "nullConst";
                break;
            case opCode::stringNew:
                ss << "stringNew " << opargs.indexv;
                break;
            case opCode::arrayNew:
                ss << "arrayNew " << opargs.indexv;
                break;
            case opCode::objectNew:
                ss << "objectNew " << opargs.indexv;
                break;
            case opCode::funcNew:
                ss << "funcNew " << opargs.indexv;
                break;
            case opCode::lambdaNew:
                ss << "lambdaNew";
                break;
            case opCode::putIndex:
                ss << "putIndex " << opargs.indexv;
                break;
            case opCode::duplicate:
                ss << "duplicate " << opargs.indexv;
                break;
            case opCode::deepCopy:
                ss << "deepCopy";
                break;
            case opCode::createOrAssign:
                ss << "createOrAssign " << opargs.indexv;
                break;
            case opCode::opThrow:
                ss << "throw";
                break;
            case opCode::popTop:
                ss << "popTop";
                break;
            case opCode::forEach:
                ss << "forEach " << opargs.intv;
                break;
            default:
                break;
        }
        return ss.str();
    }

    runtimeSourceFileMsg::operator vstr() {
        return L"at " + file + L" near line " + std::to_wstring(line) + L" column " + std::to_wstring(col);
    }
}