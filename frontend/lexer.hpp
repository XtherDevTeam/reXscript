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
                kFor,
                kForEach,
                kWhile,
                kContinue,
                kBreak,
                kReturn,
                kFunc,
                kObject,
                kClosure,
                kVar,
                kImport,
                kAs,
                kIn,
                kIf,
                kElse,
                eof,
            } kind;
            vstr value;
        };

        struct lexerState {
            vsize line, col, pos;
            vchar curCh;
            token curToken;
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

        token leftParenthesesStart();

        token rightParenthesesStart();

        token leftBracketStart();

        token rightBracketStart();

        token leftBracesStart();

        token rightBracesStart();
    };

} // rex

#endif //REXSCRIPT_LEXER_HPP
