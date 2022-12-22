//
// Created by XIaokang00010 on 2022/12/2.
//

#ifndef REXSCRIPT_SIGNALEXCEPTION_HPP
#define REXSCRIPT_SIGNALEXCEPTION_HPP

#include "interpreterSignal.hpp"
#include "interpreter/value.hpp"

namespace rex {

    class signalException : interpreterSignal {
        value msg;
    public:
        signalException(const value &v);

        value &get();
    };

} // rex

#endif //REXSCRIPT_SIGNALEXCEPTION_HPP
