//
// Created by XIaokang00010 on 2022/11/27.
//

#ifndef REXSCRIPT_HEAP_HPP
#define REXSCRIPT_HEAP_HPP

#include "share/share.hpp"

namespace rex {
    class heap {
    public:
        template<typename T>
        struct managedPtr {
            struct base {
                rex::vsize refCount;
                T v;

                base() : refCount(0), v() {}

                base(T &v) : refCount(0), v(v) {}

                base(const T &v) : refCount(0), v(v) {}
            } *ptr;


            managedPtr<T>(T &v) {
                ptr = new base(v);
                ptr->refCount++;
            }

            managedPtr<T>(const T &v) {
                ptr = new base(v);
                ptr->refCount++;
            }

            managedPtr<T>(managedPtr<T> &v) {
                ptr = v.ptr;
                ptr->refCount++;
            }

            managedPtr<T>(const managedPtr<T> &v) {
                ptr = v.ptr;
                ptr->refCount++;
            }

            ~managedPtr<T>() {
                ptr->refCount--;
                if (!ptr->refCount) {
                    delete ptr;
                }
            }

            T& operator()() {
                return ptr->v;
            }
        };
    };
}

#endif