//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_PARSEREXCEPTION_HPP
#define REXSCRIPT_PARSEREXCEPTION_HPP

#include "rexException.hpp"

namespace rex {

    class parserException : public rexException {
    public:
        parserException(vsize line, vsize col, const vstr &reason);
    };

} // rex

#endif //REXSCRIPT_PARSEREXCEPTION_HPP
