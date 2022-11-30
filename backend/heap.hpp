//
// Created by XIaokang00010 on 2022/11/27.
//

#ifndef REXSCRIPT_HEAP_HPP
#define REXSCRIPT_HEAP_HPP

#include "backend/value.hpp"
#include "share/share.hpp"

namespace rex {
    class heap {
    public:
        struct vItem {
            vItem *prev;
            vItem *next;

            bool marked;

            enum class vKind {
                vNull,
                vVec,
                vStr,
                vVal,
            } kind;

            union vValue {
                value *valPtr;
                vec<value> *vecPtr;
                vstr *strPtr;

                vValue(vec<value> *v);

                vValue(vstr *v);

                vValue(value *v);

                vValue();
            } val;

            vItem();

            vItem(vItem *prev, vKind kind, vValue val);
        } *start, *end;

        heap();

        vItem *createVec();

        vItem *createVal();

        vItem *createStr();

        void remove(vItem * v);
    };
}

#endif