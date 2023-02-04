//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_LEXER_HPP
#define REXSCRIPT_LEXER_HPP

#include <share/share.hpp>
#include <istream>

namespace rex {

    class lexer {
        std::wistream &stream;
    public:
        struct token {
            vsize line, col;
            enum class tokenKind {
                identifier,
                string,
                integer,
                decimal,
                boolean,
                toSign,
                plus,
                minus,
                asterisk,
                slash,
                percentSign,
                binaryXor,
                binaryOr,
                binaryAnd,
                logicNot,
                incrementSign,
                decrementSign,
                binaryShiftLeft,
                binaryShiftRight,
                additionAssignment,
                subtractionAssignment,
                multiplicationAssignment,
                divisionAssignment,
                reminderAssignment,
                greaterThan,
                lessThan,
                greaterEqual,
                lessEqual,
                equal,
                notEqual,
                logicAnd,
                logicOr,
                assignSign,
                leftParentheses,
                rightParentheses,
                leftBracket,
                rightBracket,
                leftBraces,
                rightBraces,
                semicolon,  // ;
                colon,      // :
                comma,      // ,
                dot,        // .
                sharp,      // #
                kFor,
                kForEach,
                kWhile,
                kContinue,
                kBreak,
                kReturn,
                kFunc,
                kClosure,
                kLet,
                kAs,
                kIn,
                kIf,
                kElse,
                kThrow,
                kTry,
                kCatch,
                kNull,
                kWith,
                eof,
            } kind;

            union vBasicValue {
                vint vInt;
                vdeci vDeci;
                vbool vBool;

                vBasicValue(vint v);

                vBasicValue(vdeci v);

                vBasicValue(vbool v);

                vBasicValue();
            } basicVal;

            vstr strVal;

            token();

            token(vsize line, vsize col, tokenKind kind);

            token(vsize line, vsize col, tokenKind kind, vBasicValue basicVal);

            token(vsize line, vsize col, tokenKind kind, vstr strVal);
        };

        struct lexerState {
            vsize line, col;
            std::istream::pos_type pos;
            vchar curCh;
            token curToken;

            lexerState();

            lexerState(vsize line, vsize col, std::istream::pos_type pos, vchar curCh, lexer::token curToken);
        };

        vec<lexerState> states;

        token curToken;
        vchar curCh;

        vsize line, col;

        void getCh();

        explicit lexer(std::wistream &ss);

        void saveState();

        void returnState();

        void dropState();

        token scan();

        token alphaStart();

        token strStart();

        token digitStart();

        token minusStart();

        token plusStart();

        token asteriskStart();

        token slashStart();

        token percentSignStart();

        token equalStart();

        token notStart();

        token lessStart();

        token greaterStart();

        token semicolonStart();

        token colonStart();

        token commaStart();

        token dotStart();

        token sharpStart();

        token leftParenthesesStart();

        token rightParenthesesStart();

        token leftBracketStart();

        token rightBracketStart();

        token leftBracesStart();

        token rightBracesStart();

        token andStart();

        token orStart();

        token xorStart();
    };

} // rex

#endif //REXSCRIPT_LEXER_HPP
