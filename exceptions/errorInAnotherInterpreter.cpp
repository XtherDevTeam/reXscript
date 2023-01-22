//
// Created by XIaokang00010 on 2023/1/22.
//

#include "errorInAnotherInterpreter.hpp"

namespace rex {
    errorInAnotherInterpreter::errorInAnotherInterpreter() : rexException(L"error in another interpreter") {

    }

    const char *errorInAnotherInterpreter::what() const noexcept {
        return rexException::what();
    }
} // rex