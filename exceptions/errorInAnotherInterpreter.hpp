//
// Created by XIaokang00010 on 2023/1/22.
//

#ifndef REXSCRIPT_ERRORINANOTHERINTERPRETER_HPP
#define REXSCRIPT_ERRORINANOTHERINTERPRETER_HPP

#include "rexException.hpp"

namespace rex {

    class errorInAnotherInterpreter : public rexException {
    public:
        errorInAnotherInterpreter();

        const char * what() const noexcept override;
    };

} // rex

#endif //REXSCRIPT_ERRORINANOTHERINTERPRETER_HPP
