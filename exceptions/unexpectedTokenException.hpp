//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_UNEXPECTEDTOKENEXCEPTION_HPP
#define REXSCRIPT_UNEXPECTEDTOKENEXCEPTION_HPP

#include "rexException.hpp"

namespace rex {

    class unexpectedTokenException : public rexException {
    public:
        unexpectedTokenException(vsize line, vsize col, vchar ch);
    };

} // rex

#endif //REXSCRIPT_UNEXPECTEDTOKENEXCEPTION_HPP
