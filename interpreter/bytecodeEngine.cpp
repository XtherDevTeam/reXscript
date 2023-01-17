//
// Created by XIaokang00010 on 2023/1/14.
//

#include "bytecodeEngine.hpp"
#include "exceptions/signalException.hpp"
#include "builtInMethods.hpp"

#include <utility>

namespace rex::bytecodeEngine {
    managedPtr<environment> rexEnvironmentInstance = managePtr(environment{});

    void environment::stackFrame::pushLocalCxt(const value::cxtObject &cxt) {
        localCxt.push_back(cxt);
    }

    void environment::stackFrame::popLocalCxt() {
        localCxt.pop_back();
    }

    environment::stackFrame::stackFrame() : moduleCxt(nullptr), localCxt() {

    }

    environment::stackFrame::stackFrame(const managedPtr<value> &moduleCxt, const vec<value::cxtObject> &localCxt,
                                        codeStruct *code)
            : moduleCxt(moduleCxt), localCxt(localCxt), currentCodeStruct(code) {

    }

    vsize environment::stackFrame::getCurLocalCxtIdx() {
        return localCxt.size();
    }

    void environment::stackFrame::backToLocalCxt(vsize idx) {
        while (localCxt.size() > idx)
            localCxt.pop_back();
    }

    environment::stackFrame::operator vstr() {
        return L"stack frame at " + (vstr) currentCodeStruct->msg;
    }

    environment::thread::thread() : th(), result() {
    }

    void environment::thread::setTh(const managedPtr<std::thread> &v) {
        th = v;
    }

    void environment::thread::setResult(const managedPtr<value> &v) {
        result = v;
    }

    const managedPtr<std::thread> &environment::thread::getTh() {
        return th;
    }

    const managedPtr<value> &environment::thread::getResult() {
        return result;
    }

    codeBuilder::codeBuilder(bytecodeModule mod, codeStruct &currentBlock) : mod(std::move(mod)), currentBlock(
            currentBlock) {

    }

    uint64_t codeBuilder::getNextCur() {
        return (uint64_t) (currentBlock.code.end() - currentBlock.code.begin());
    }

    void codeBuilder::buildBasicLiterals(const AST &target) {
        switch (target.leaf.kind) {
            case lexer::token::tokenKind::string:
                currentBlock.code.push_back(
                        {bytecodeStruct::opCode::stringNew, {currentBlock.putStringConst(target.leaf.strVal)}});
                break;
            case lexer::token::tokenKind::integer:
                currentBlock.code.push_back({bytecodeStruct::opCode::intConst, {target.leaf.basicVal.vInt}});
                break;
            case lexer::token::tokenKind::decimal:
                currentBlock.code.push_back({bytecodeStruct::opCode::deciConst, {target.leaf.basicVal.vDeci}});
                break;
            case lexer::token::tokenKind::boolean:
                currentBlock.code.push_back({bytecodeStruct::opCode::boolConst, {target.leaf.basicVal.vBool}});
                break;
            case lexer::token::tokenKind::kNull:
                currentBlock.code.push_back({bytecodeStruct::opCode::nullConst, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildIdentifier(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::find, {currentBlock.putNames(target.leaf.strVal)}});
    }

    void codeBuilder::buildSubscriptExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[1]);
        currentBlock.code.push_back({bytecodeStruct::opCode::index, {}});
    }

    void codeBuilder::buildInvokeExpr(const AST &target) {
        buildExpr(target.child[0]);
        std::for_each(target.child[1].child.rbegin(), target.child[1].child.rend(), [&](const AST &ast) {
            buildExpr(ast);
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::invoke, {(uint64_t) target.child[1].child.size()}});
    }

    void codeBuilder::buildListLiteral(const AST &target) {
        std::for_each(target.child.rbegin(), target.child.rend(), [&](const AST &ast) {
            buildExpr(ast);
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::arrayNew, {(uint64_t) target.child.size()}});
    }

    void codeBuilder::buildObjectLiteral(const AST &target) {
        std::for_each(target.child.rbegin(), target.child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back(
                    {bytecodeStruct::opCode::putIndex, {currentBlock.putStringConst(ast.child[0].leaf.strVal)}});
            buildExpr(ast.child[1]);
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::objectNew, {(uint64_t) target.child.size()}});
    }

    void codeBuilder::buildLambdaDef(const AST &target) {
        std::for_each(target.child[0].child.rbegin(), target.child[0].child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back(
                    {bytecodeStruct::opCode::putIndex, {currentBlock.putStringConst(ast.leaf.strVal)}});
            currentBlock.code.push_back({bytecodeStruct::opCode::find, {currentBlock.putNames(ast.leaf.strVal)}});
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::objectNew, {(uint64_t) target.child[0].child.size()}});

        std::for_each(target.child[1].child.rbegin(), target.child[1].child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {currentBlock.putNames(ast.leaf.strVal)}});
        });
        // build function body
        uint64_t func = mod.putCodeStruct({});
        codeBuilder cb(mod, *mod.codeStructs[func]);
        cb.buildStmt(target.child[2]);
        currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {func}});
        currentBlock.code.push_back({bytecodeStruct::opCode::funcNew, {(uint64_t) target.child[1].child.size()}});
        currentBlock.code.push_back({bytecodeStruct::opCode::lambdaNew, {}});
    }

    void codeBuilder::buildFuncDef(const AST &target) {
        std::for_each(target.child[0].child.rbegin(), target.child[0].child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {currentBlock.putNames(ast.leaf.strVal)}});
        });
        // build function body
        uint64_t func = mod.putCodeStruct({});
        codeBuilder cb(mod, *mod.codeStructs[func]);
        cb.buildStmt(target.child[1]);
        currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {func}});
        currentBlock.code.push_back({bytecodeStruct::opCode::funcNew, {(uint64_t) target.child[0].child.size()}});
    }

    void codeBuilder::buildMemberExpr(const AST &target) {
        buildExpr(target.child[0]);
        switch (target.child[1].kind) {
            case AST::treeKind::identifier:
                currentBlock.code.push_back(
                        {bytecodeStruct::opCode::findAttr, {currentBlock.putNames(target.child[1].leaf.strVal)}});
                break;
            case AST::treeKind::subscriptExpression:
                currentBlock.code.push_back({bytecodeStruct::opCode::findAttr,
                                             {currentBlock.putNames(target.child[1].child[0].leaf.strVal)}});
                buildExpr(target.child[1].child[1]);
                currentBlock.code.push_back({bytecodeStruct::opCode::index, {}});
                break;
            case AST::treeKind::invokingExpression:
                currentBlock.code.push_back({bytecodeStruct::opCode::duplicate, {}});
                currentBlock.code.push_back({bytecodeStruct::opCode::findAttr,
                                             {currentBlock.putNames(target.child[1].child[0].leaf.strVal)}});

                std::for_each(
                        target.child[1].child[1].child.rbegin(), target.child[1].child[1].child.rend(),
                        [&](const AST &ast) {
                            buildExpr(ast);
                        });
                currentBlock.code.push_back(
                        {bytecodeStruct::opCode::invokeMethod, {(uint64_t) target.child[1].child[1].child.size()}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildUniqueExpr(const AST &target) {
        buildExpr(target.child[1]);
        switch (target.child[0].leaf.kind) {
            case lexer::token::tokenKind::minus:
                currentBlock.code.push_back({bytecodeStruct::opCode::opNegate, {}});
                break;
            case lexer::token::tokenKind::incrementSign:
                currentBlock.code.push_back({bytecodeStruct::opCode::opIncrement, {}});
                break;
            case lexer::token::tokenKind::decrementSign:
                currentBlock.code.push_back({bytecodeStruct::opCode::opDecrement, {}});
                break;
            case lexer::token::tokenKind::asterisk:
                currentBlock.code.push_back({bytecodeStruct::opCode::deepCopy, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildMulExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::asterisk:
                currentBlock.code.push_back({bytecodeStruct::opCode::opMul, {}});
                break;
            case lexer::token::tokenKind::slash:
                currentBlock.code.push_back({bytecodeStruct::opCode::opDiv, {}});
                break;
            case lexer::token::tokenKind::percentSign:
                currentBlock.code.push_back({bytecodeStruct::opCode::opMod, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildAddExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::plus:
                currentBlock.code.push_back({bytecodeStruct::opCode::opAdd, {}});
                break;
            case lexer::token::tokenKind::minus:
                currentBlock.code.push_back({bytecodeStruct::opCode::opSub, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildBinShiftExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::binaryShiftLeft:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryShiftLeft, {}});
                break;
            case lexer::token::tokenKind::binaryShiftRight:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryShiftRight, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildLogicEqExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::equal:
                currentBlock.code.push_back({bytecodeStruct::opCode::opEqual, {}});
                break;
            case lexer::token::tokenKind::notEqual:
                currentBlock.code.push_back({bytecodeStruct::opCode::opNotEqual, {}});
                break;
            case lexer::token::tokenKind::greaterEqual:
                currentBlock.code.push_back({bytecodeStruct::opCode::opGreaterEqual, {}});
                break;
            case lexer::token::tokenKind::lessEqual:
                currentBlock.code.push_back({bytecodeStruct::opCode::opLessEqual, {}});
                break;
            case lexer::token::tokenKind::greaterThan:
                currentBlock.code.push_back({bytecodeStruct::opCode::opGreaterThan, {}});
                break;
            case lexer::token::tokenKind::lessThan:
                currentBlock.code.push_back({bytecodeStruct::opCode::opLessThan, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildBinExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::binaryOr:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryOr, {}});
                break;
            case lexer::token::tokenKind::binaryAnd:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryAnd, {}});
                break;
            case lexer::token::tokenKind::binaryXor:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryXor, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildLogicAndExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::logicAnd:
                currentBlock.code.push_back({bytecodeStruct::opCode::opLogicAnd, {}});
                break;
            case lexer::token::tokenKind::logicOr:
                currentBlock.code.push_back({bytecodeStruct::opCode::opLogicOr, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildAssignmentExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);
        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::assignSign:
                currentBlock.code.push_back({bytecodeStruct::opCode::assign, {}});
                break;
            case lexer::token::tokenKind::additionAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::addAssign, {}});
                break;
            case lexer::token::tokenKind::subtractionAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::subAssign, {}});
                break;
            case lexer::token::tokenKind::multiplicationAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::mulAssign, {}});
                break;
            case lexer::token::tokenKind::divisionAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::divAssign, {}});
                break;
            case lexer::token::tokenKind::reminderAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::modAssign, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildBlockStmt(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::pushLocalCxt, {}});
        std::for_each(target.child.begin(), target.child.end(), [&](const AST &ast) {
            buildStmt(ast);
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::popLocalCxt, {}});
    }

    void codeBuilder::buildLetStmt(const AST &target) {
        std::for_each(target.child.begin(), target.child.end(), [&](const AST &ast) {
            buildExpr(ast.child[1]);
            currentBlock.code.push_back(
                    {bytecodeStruct::opCode::createOrAssign, {currentBlock.putNames(ast.child[0].leaf.strVal)}});
        });
    }

    void codeBuilder::buildWhileStmt(const AST &target) {
        uint64_t begin = getNextCur();
        buildExpr(target.child[0]);
        uint64_t jumpOutOfLoopWithCond = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jumpIfFalse, {(vint) 114514}});
        buildStmt(target.child[1]);
        uint64_t jumpBack = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jump, {(vint) (begin - jumpBack)}});
        uint64_t end = getNextCur();
        currentBlock.code[jumpOutOfLoopWithCond].opargs = (vint) (end - jumpOutOfLoopWithCond);
        uint64_t curIdx = 0;
        std::for_each(currentBlock.code.begin(), currentBlock.code.end(), [&](auto &i) {
            if (i.opcode == bytecodeStruct::opCode::fakeOpBreak)
                i = {bytecodeStruct::opCode::jump, (vint) (end - curIdx)};
            if (i.opcode == bytecodeStruct::opCode::fakeOpContinue)
                i = {bytecodeStruct::opCode::jump, (vint) (jumpBack - curIdx)};
            curIdx++;
        });
    }

    void codeBuilder::buildForStmt(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::pushLocalCxt, {}});
        // init stmt
        buildStmt(target.child[0]);
        // cond stmt
        uint64_t begin = getNextCur();
        buildExpr(target.child[1]);
        uint64_t jumpOutOfLoopWithCond = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jumpIfFalse, {(vint) 114514}});
        // block stmt
        buildStmt(target.child[3]);
        uint64_t endBlock = getNextCur();
        buildStmt(target.child[2]);
        uint64_t jumpBack = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jump, {(vint) 114514}});
        uint64_t end = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::popLocalCxt, {}});
        // fill the jump positions
        currentBlock.code[jumpOutOfLoopWithCond].opargs = (vint) (end - jumpOutOfLoopWithCond);
        currentBlock.code[jumpBack].opargs = (vint) (begin - jumpBack);
        // replace fake op
        uint64_t curIdx = 0;
        std::for_each(currentBlock.code.begin(), currentBlock.code.end(), [&](auto &i) {
            if (i.opcode == bytecodeStruct::opCode::fakeOpBreak)
                i = {bytecodeStruct::opCode::jump, (vint) (end - curIdx)};
            if (i.opcode == bytecodeStruct::opCode::fakeOpContinue)
                i = {bytecodeStruct::opCode::jump, (vint) (jumpBack - curIdx)};
            curIdx++;
        });
    }

    void codeBuilder::buildForEachStmt(const AST &target) {
        // leave it empty
    }

    void codeBuilder::buildIfStmt(const AST &target) {
        buildExpr(target.child[0]);
        uint64_t jumpStmt = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jumpIfFalse, {(vint) 114514}});
        buildStmt(target.child[1]);
        uint64_t end = getNextCur();
        currentBlock.code[jumpStmt].opargs = (vint) (end - jumpStmt);
    }

    void codeBuilder::buildIfElseStmt(const AST &target) {
        buildExpr(target.child[0]);
        uint64_t jumpStmt = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jumpIfFalse, {(vint) 114514}});
        buildStmt(target.child[1]);
        uint64_t jumpOutOfElse = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jump, {(vint) 114514}});
        uint64_t elseBlock = getNextCur();
        buildStmt(target.child[2]);
        uint64_t end = getNextCur();

        currentBlock.code[jumpStmt].opargs = (vint) (elseBlock - jumpStmt);
        currentBlock.code[jumpOutOfElse].opargs = (vint) (end - jumpOutOfElse);
    }

    void codeBuilder::buildReturnStmt(const AST &target) {
        buildExpr(target.child[0]);
        currentBlock.code.push_back({bytecodeStruct::opCode::ret, {}});
    }

    void codeBuilder::buildContinueStmt(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::fakeOpContinue, {}});
    }

    void codeBuilder::buildBreakStmt(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::fakeOpBreak, {}});
    }

    void codeBuilder::buildTryCatchStmt(const AST &target) {
        uint64_t pushEventHandler = getNextCur();
        // try
        currentBlock.code.push_back({bytecodeStruct::opCode::pushExceptionHandler, {(vint) 114514}});
        buildStmt(target.child[0]);
        currentBlock.code.push_back({bytecodeStruct::opCode::popExceptionHandler, {}});
        uint64_t jumpOut = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jump, {(vint) 114514}});
        // catch
        uint64_t catchBlock = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::pushLocalCxt, {}});
        currentBlock.code.push_back(
                {bytecodeStruct::opCode::createOrAssign, {currentBlock.putNames(target.child[1].leaf.strVal)}});
        buildStmt(target.child[2]);
        currentBlock.code.push_back({bytecodeStruct::opCode::popLocalCxt, {}});
        // fill the addresses
        uint64_t end = getNextCur();
        currentBlock.code[pushEventHandler].opargs = (vint) (catchBlock - pushEventHandler);
        currentBlock.code[jumpOut].opargs = (vint) (end - jumpOut);
    }

    void codeBuilder::buildThrowStmt(const AST &target) {
        buildExpr(target.child[0]);
        currentBlock.code.push_back({bytecodeStruct::opCode::opThrow, {}});
    }

    void codeBuilder::buildFuncDefStmt(const AST &target) {
        const vstr &funcName = target.child[0].leaf.strVal;
        std::for_each(target.child[1].child.rbegin(), target.child[1].child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {currentBlock.putNames(ast.leaf.strVal)}});
        });
        codeBuilder cb(mod, *mod.codeStructs[mod.putCodeStruct({})]);
        cb.buildStmt(target.child[2]);
        currentBlock.code.push_back({bytecodeStruct::opCode::funcNew, {(uint64_t) target.child[1].child.size()}});
        currentBlock.code.push_back({bytecodeStruct::opCode::createOrAssign, {currentBlock.putNames(funcName)}});
    }

    void codeBuilder::buildStmt(const AST &target) {
        switch (target.kind) {
            case AST::treeKind::identifier:
            case AST::treeKind::subscriptExpression:
            case AST::treeKind::invokingExpression:
            case AST::treeKind::listLiteral:
            case AST::treeKind::objectLiteral:
            case AST::treeKind::lambdaDefinition:
            case AST::treeKind::functionDefinition:
            case AST::treeKind::memberExpression:
            case AST::treeKind::uniqueExpression:
            case AST::treeKind::multiplicationExpression:
            case AST::treeKind::additionExpression:
            case AST::treeKind::binaryShiftExpression:
            case AST::treeKind::logicEqualExpression:
            case AST::treeKind::binaryExpression:
            case AST::treeKind::logicAndExpression:
                buildExpr(target);
                currentBlock.code.push_back({bytecodeStruct::opCode::popTop, {}});
                break;
            case AST::treeKind::assignmentExpression:
                buildAssignmentExpr(target);
                break;
            case AST::treeKind::blockStmt:
                buildBlockStmt(target);
                break;
            case AST::treeKind::letStmt:
                buildLetStmt(target);
                break;
            case AST::treeKind::whileStmt:
                buildWhileStmt(target);
                break;
            case AST::treeKind::forStmt:
                buildForStmt(target);
                break;
            case AST::treeKind::rangeBasedForStmt:
                buildForEachStmt(target);
                break;
            case AST::treeKind::ifStmt:
                buildIfStmt(target);
                break;
            case AST::treeKind::ifElseStmt:
                buildIfElseStmt(target);
                break;
            case AST::treeKind::returnStmt:
                buildReturnStmt(target);
                break;
            case AST::treeKind::continueStmt:
                buildContinueStmt(target);
                break;
            case AST::treeKind::breakStmt:
                buildBreakStmt(target);
                break;
            case AST::treeKind::tryCatchStmt:
                buildTryCatchStmt(target);
                break;
            case AST::treeKind::throwStmt:
                buildThrowStmt(target);
                break;
            case AST::treeKind::functionDefStmt:
                buildFuncDefStmt(target);
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildExpr(const AST &target) {
        switch (target.kind) {
            case AST::treeKind::identifier:
                buildIdentifier(target);
                break;
            case AST::treeKind::subscriptExpression:
                buildSubscriptExpr(target);
                break;
            case AST::treeKind::invokingExpression:
                buildInvokeExpr(target);
                break;
            case AST::treeKind::basicLiterals:
                buildBasicLiterals(target);
                break;
            case AST::treeKind::listLiteral:
                buildListLiteral(target);
                break;
            case AST::treeKind::objectLiteral:
                buildObjectLiteral(target);
                break;
            case AST::treeKind::lambdaDefinition:
                buildLambdaDef(target);
                break;
            case AST::treeKind::functionDefinition:
                buildFuncDef(target);
                break;
            case AST::treeKind::memberExpression:
                buildMemberExpr(target);
                break;
            case AST::treeKind::uniqueExpression:
                buildUniqueExpr(target);
                break;
            case AST::treeKind::multiplicationExpression:
                buildMulExpr(target);
                break;
            case AST::treeKind::additionExpression:
                buildAddExpr(target);
                break;
            case AST::treeKind::binaryShiftExpression:
                buildBinShiftExpr(target);
                break;
            case AST::treeKind::logicEqualExpression:
                buildLogicEqExpr(target);
                break;
            case AST::treeKind::binaryExpression:
                buildBinExpr(target);
                break;
            case AST::treeKind::logicAndExpression:
                buildLogicAndExpr(target);
                break;
            default:
                break;
        }
    }

    interpreter::interpreter(const managedPtr<environment> &env, const managedPtr<value> &interpreterCxt,
                             const managedPtr<value> &moduleCxt) :
            env(env), interpreterCxt(interpreterCxt), moduleCxt(moduleCxt) {
        callStack.push_back({moduleCxt, {}, nullptr});
    }


    value interpreter::opAdd(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() + b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() + (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() + b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() + b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() + (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() + b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() + (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() + (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() + b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexAdd"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opSub(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() - b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() - (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() - b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() - b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() - (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() - b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() - (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() - (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() - b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexSub"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opMul(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() * b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() * (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() * b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() * b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() * (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() * b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() * (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() * (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() * b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexMul"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opDiv(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() / b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() / (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() / b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() / b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() / (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() / b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() / (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() / (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() / b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexDiv"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opMod(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() % b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() % (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() % b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() % (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexAdd"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opBinaryShiftLeft(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() << b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() << (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() << b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() << (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryShiftLeft"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opBinaryShiftRight(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() >> b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() >> (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() >> b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() >> (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryShiftLeft"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opEqual(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() == b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() == (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() == b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() == b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() == (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() == b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() == (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() == (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() == b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexEqual"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opNotEqual(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() != b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() != (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() != b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() != b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() != (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() != b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() != (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() != (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() != b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexNotEqual"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opGreaterEqual(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() >= b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() >= (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() >= b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() >= b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() >= (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() >= b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() >= (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() >= (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() >= b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexGreaterEqual"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opLessEqual(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() <= b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() <= (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() <= b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() <= b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() <= (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() <= b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() <= (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() <= (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() <= b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexLessEqual"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opGreaterThan(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() > b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() > (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() > b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() > b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() > (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() > b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() > (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() > (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() > b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexGreaterThan"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opLessThan(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() < b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() < (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() < b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() < b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() < (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() < b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() < (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() < (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() < b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexLessThan"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opBinaryOr(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() | b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() | (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() | b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() | (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryOr"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opBinaryAnd(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() & b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() & (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() & b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() & (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryAnd"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opBinaryXor(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() ^ b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() ^ (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() ^ b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() ^ (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryXor"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opLogicAnd(value &a, value &b) {
        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() && b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() && (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() && b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() && (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexLogicAnd"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opLogicOr(value &a, value &b) {
        if (a.isRef())
            a = a.getRef();
        if (b.isRef())
            b = b.getRef();

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() || b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() || (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() || b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() || (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexLogicOr"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throwErr(makeErr(L"typeError", L"unsupported operation"));
            }
        }
        return {};
    }

    value interpreter::opIncrement(value &a) {
        switch (a.kind) {
            case value::vKind::vInt: {
                a.getInt()++;
                return a;
            }
            case value::vKind::vDeci: {
                a.getDeci()++;
                return a;
            }
            case value::vKind::vObject: {

            }
        }
    }

    value interpreter::opDecrement(value &a) {
        return value();
    }

    value interpreter::makeErr(const vstr &errName, const vstr &errMsg) {
        value res{(value::cxtObject) {}};
//        res.members[L"errName"] = managePtr(value{errName, stringMethods::getMethodsCxt()});
//        res.members[L"errMsg"] = managePtr(value{errMsg, stringMethods::getMethodsCxt()}); // add string initialization
        return res;
    }

    void interpreter::interpret() {
        interpret:
        try {
            while (callStack.back().programCounter < callStack.back().currentCodeStruct->code.size()) {
                execute(callStack.back().currentCodeStruct->code[callStack.back().programCounter]);
            }
        } catch (signalException &e) {
            if (exceptionHandlers.empty() or exceptionHandlers.back().frame + 1 != evalStack.size())
                throw;
            restoreState(exceptionHandlers.back());
            exceptionHandlers.pop_back();
            evalStack.push_back(e.get());
            goto interpret;
        }
    }

    void interpreter::throwErr(const value &err) {
        throw signalException(err);
    }

    value interpreter::invokeFunc(const managedPtr<value> &func, const vec<value> &args,
                                  const managedPtr<value> &passThisPtr) {
        return {};
    }

    void interpreter::restoreState(const interpreter::state &s) {
        while (s.evalStack != evalStack.size()) evalStack.pop_back();
        while (s.frame != callStack.size()) callStack.pop_back();
        while (s.localCxt != callStack.back().localCxt.size()) callStack.back().localCxt.pop_back();
        callStack.back().programCounter = s.program;
    }

    void interpreter::execute(const bytecodeStruct &op) {
        switch (op.opcode) {
            case bytecodeStruct::opCode::pushLocalCxt:
                callStack.back().localCxt.emplace_back();
                break;
            case bytecodeStruct::opCode::popLocalCxt:
                callStack.back().localCxt.pop_back();
                break;
            case bytecodeStruct::opCode::jump: {
                callStack.back().programCounter += op.opargs.intv;
                break;
            }
            case bytecodeStruct::opCode::jumpIfTrue: {
                if (evalStack.back().getBool()) {
                    callStack.back().programCounter += op.opargs.intv;
                    evalStack.pop_back();
                } else {
                    evalStack.pop_back();
                }
                break;
            }
            case bytecodeStruct::opCode::jumpIfFalse: {
                if (!evalStack.back().getBool()) {
                    callStack.back().programCounter += op.opargs.intv;
                    evalStack.pop_back();
                } else {
                    evalStack.pop_back();
                }
                break;
            }
            case bytecodeStruct::opCode::pushExceptionHandler:
                exceptionHandlers.push_back(
                        {(uint64_t) callStack.size(), (uint64_t) callStack.back().localCxt.size(),
                         (uint64_t) (callStack.back().programCounter + op.opargs.intv), (uint64_t) evalStack.size()});
                break;
            case bytecodeStruct::opCode::popExceptionHandler:
                exceptionHandlers.pop_back();
                break;
            case bytecodeStruct::opCode::invoke:
                break;
            case bytecodeStruct::opCode::ret: {
                callStack.pop_back();
                break;
            }
            case bytecodeStruct::opCode::find: {
                vstr &str = callStack.back().currentCodeStruct->names[op.opargs.indexv];
                for (auto iter = callStack.back().localCxt.begin(); iter != callStack.back().localCxt.end(); iter++) {
                    if (auto it = iter->find(str); it != iter->end()) {
                        // TODO: do sth
                    }
                }
                if (auto it = callStack.back().moduleCxt->members.find(str);
                        it != callStack.back().moduleCxt->members.end()) {
                    // TODO: do sth
                }
                if (auto it = interpreterCxt->members.find(str); it != interpreterCxt->members.end()) {
                    // TODO: do sth
                }
                break;
            }
            case bytecodeStruct::opCode::findAttr: {
                vstr &str = callStack.back().currentCodeStruct->names[op.opargs.indexv];
                value &back = evalStack.back().isRef() ? evalStack.back().getRef() : evalStack.back();
                if (auto it = back.members.find(str); it != back.members.end()) {
                    // TODO: do sth
                }
                break;
            }
            case bytecodeStruct::opCode::index:
                break;
            case bytecodeStruct::opCode::invokeMethod:
                break;
            case bytecodeStruct::opCode::opIncrement:
                break;
            case bytecodeStruct::opCode::opDecrement:
                break;
            case bytecodeStruct::opCode::opNegate:
                break;
            case bytecodeStruct::opCode::opAdd:
                break;
            case bytecodeStruct::opCode::opSub:
                break;
            case bytecodeStruct::opCode::opMul:
                break;
            case bytecodeStruct::opCode::opDiv:
                break;
            case bytecodeStruct::opCode::opMod:
                break;
            case bytecodeStruct::opCode::opBinaryShiftLeft:
                break;
            case bytecodeStruct::opCode::opBinaryShiftRight:
                break;
            case bytecodeStruct::opCode::opEqual:
                break;
            case bytecodeStruct::opCode::opNotEqual:
                break;
            case bytecodeStruct::opCode::opGreaterEqual:
                break;
            case bytecodeStruct::opCode::opLessEqual:
                break;
            case bytecodeStruct::opCode::opGreaterThan:
                break;
            case bytecodeStruct::opCode::opLessThan:
                break;
            case bytecodeStruct::opCode::opBinaryOr:
                break;
            case bytecodeStruct::opCode::opBinaryAnd:
                break;
            case bytecodeStruct::opCode::opBinaryXor:
                break;
            case bytecodeStruct::opCode::opLogicAnd:
                break;
            case bytecodeStruct::opCode::opLogicOr:
                break;
            case bytecodeStruct::opCode::assign:
                break;
            case bytecodeStruct::opCode::addAssign:
                break;
            case bytecodeStruct::opCode::subAssign:
                break;
            case bytecodeStruct::opCode::mulAssign:
                break;
            case bytecodeStruct::opCode::divAssign:
                break;
            case bytecodeStruct::opCode::modAssign:
                break;
            case bytecodeStruct::opCode::intConst: {
                evalStack.emplace_back(op.opargs.intv);
                break;
            }
            case bytecodeStruct::opCode::deciConst: {
                evalStack.emplace_back(op.opargs.deciv);
                break;
            }
            case bytecodeStruct::opCode::boolConst: {
                evalStack.emplace_back(op.opargs.boolv);
                break;
            }
            case bytecodeStruct::opCode::nullConst: {
                evalStack.emplace_back();
                break;
            }
            case bytecodeStruct::opCode::stringNew: {
                evalStack.emplace_back(callStack.back().currentCodeStruct->stringConsts[op.opargs.indexv],
                                       stringMethods::getMethodsCxt());
                break;
            }
            case bytecodeStruct::opCode::arrayNew: {
                value::vecObject object;
                auto i = op.opargs.indexv;
                while (i--) {
                    object.push_back(evalStack.back().isRef() ? evalStack.back().refObj : managePtr(evalStack.back()));
                    evalStack.pop_back();
                }
                evalStack.emplace_back(object, vecMethods::getMethodsCxt());
                break;
            }
            case bytecodeStruct::opCode::objectNew: {
                value::cxtObject object;
                auto i = op.opargs.indexv;
                while (i--) {
                    managedPtr<value> v = evalStack.back().isRef() ? evalStack.back().refObj : managePtr(
                            evalStack.back());
                    evalStack.pop_back();
                    object[callStack.back().currentCodeStruct->stringConsts[(uint64_t) evalStack.back().basicValue.unknown]] = v;
                }
                evalStack.emplace_back(object);
                break;
            }
            case bytecodeStruct::opCode::funcNew: {
                value::funcObject object;
                object.code = getBytecodeModule().codeStructs[(uint64_t) evalStack.back().basicValue.unknown];
                evalStack.pop_back();

                auto i = op.opargs.indexv;
                while (i--) {
                    object.argsName.push_back(
                            callStack.back().currentCodeStruct->names[(uint64_t) evalStack.back().basicValue.unknown]);
                    evalStack.pop_back();
                }
                evalStack.emplace_back(object);
                break;
            }
            case bytecodeStruct::opCode::lambdaNew: {
                value::lambdaObject object;
                object.func = evalStack.back().getFunc();
                evalStack.pop_back();
                object.outerCxt = managePtr(evalStack.back());
                evalStack.pop_back();
                break;
            }
            case bytecodeStruct::opCode::putIndex: {
                evalStack.emplace_back((unknownPtr) op.opargs.indexv);
                break;
            }
            case bytecodeStruct::opCode::duplicate: {
                evalStack.push_back(evalStack.back());
                break;
            }
            case bytecodeStruct::opCode::deepCopy: {
                value dest;
                (evalStack.back().isRef() ? evalStack.back().getRef() : evalStack.back()).deepCopy(dest);
                evalStack.pop_back();
                evalStack.push_back(dest);
                break;
            }
            case bytecodeStruct::opCode::createOrAssign: {
                callStack.back().localCxt.back()[callStack.back().currentCodeStruct->names[op.opargs.indexv]] =
                        evalStack.back().isRef() ? evalStack.back().refObj : managePtr(evalStack.back());
                break;
            }
            case bytecodeStruct::opCode::opThrow: {
                throwErr(evalStack.back());
                break;
            }
            case bytecodeStruct::opCode::popTop: {
                evalStack.pop_back();
                break;
            }
            default:
                break;
        }
    }

    bytecodeModule interpreter::getBytecodeModule() {
        return callStack.back().moduleCxt->members[L"__code__"]->getBytecodeModule();
    }
}
