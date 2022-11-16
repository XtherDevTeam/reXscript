//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_ENDOFFILEEXCEPTION_HPP
#define REXSCRIPT_ENDOFFILEEXCEPTION_HPP

#include <exceptions/rexException.hpp>

namespace rex {

    class endOfFileException : public rexException {
    public:
        endOfFileException();
    };

} // rex

#endif //REXSCRIPT_ENDOFFILEEXCEPTION_HPP
