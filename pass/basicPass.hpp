//
// Created by XIaokang00010 on 2022/11/12.
//

#ifndef REXSCRIPT_BASICPASS_HPP
#define REXSCRIPT_BASICPASS_HPP

namespace rex {

    template<class T, class T1> class basicPass {
    protected:
        T &before;
    public:
        basicPass(T &before) : before(before) {}

        virtual T1 run() = 0;
    };

} // rex

#endif //REXSCRIPT_BASICPASS_HPP
