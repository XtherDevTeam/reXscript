//
// Created by XIaokang00010 on 2022/11/15.
//

#ifndef REXSCRIPT_SIGNALRETURN_HPP
#define REXSCRIPT_SIGNALRETURN_HPP

#include "interpreterSignal.hpp"
#include "interpreter/value.hpp"

namespace rex {

    class signalReturn : public interpreterSignal {
        value msg;
    public:
        signalReturn(const value &v);

        const value &get();
    };

} // rex

#endif //REXSCRIPT_SIGNALRETURN_HPP
