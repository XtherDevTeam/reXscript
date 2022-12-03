//
// Created by XIaokang00010 on 2022/11/15.
//

#ifndef REXSCRIPT_INTERPRETERSIGNAL_HPP
#define REXSCRIPT_INTERPRETERSIGNAL_HPP

#include <exception>

namespace rex {

    class interpreterSignal : std::exception {
    public:
        interpreterSignal();

        const char * what() const noexcept override;
    };

} // rex

#endif //REXSCRIPT_INTERPRETERSIGNAL_HPP
