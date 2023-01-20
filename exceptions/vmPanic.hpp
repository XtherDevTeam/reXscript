//
// Created by XIaokang00010 on 2023/1/20.
//

#ifndef REXSCRIPT_VMPANIC_HPP
#define REXSCRIPT_VMPANIC_HPP

#include "rexException.hpp"

namespace rex {
    class vmPanic : public rexException {
    public:
        vmPanic(const vstr &reason);

        const char * what() const noexcept override;
    };
} // rex

#endif //REXSCRIPT_VMPANIC_HPP
