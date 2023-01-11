//
// Created by XIaokang00010 on 2023/1/11.
//

#ifndef REXSCRIPT_IMPORTERROR_HPP
#define REXSCRIPT_IMPORTERROR_HPP

#include "rexException.hpp"

namespace rex {

    class importError : public rexException {
    public:
        importError(const vstr& reason);

        const char * what() const noexcept override;
    };

} // rex

#endif //REXSCRIPT_IMPORTERROR_HPP
