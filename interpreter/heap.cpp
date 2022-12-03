//
// Created by XIaokang00010 on 2022/11/27.
//

#include "heap.hpp"

namespace rex {
    heap::vItem::vValue::vValue() : valPtr(nullptr) {}

    heap::vItem::vValue::vValue(value *v) : valPtr(v) {}

    heap::vItem::vValue::vValue(vec<value> *v) : vecPtr(v) {}

    heap::vItem::vValue::vValue(vstr *v) : strPtr(v) {}

    heap::vItem::vItem() : prev(nullptr), next(nullptr), kind(vKind::vNull), marked(false), val() {

    }

    heap::vItem::vItem(vItem *prev, vKind kind, vValue val) : prev(prev), next(nullptr), marked(false), kind(kind), val(val) {
        prev->next = this;
    }

    heap::vItem *heap::createStr() {
        if (start == nullptr) {
            start = end = new vItem();
            start->kind = heap::vItem::vKind::vStr;
            start->val = new vstr();
        } else {
            end = new vItem(end, heap::vItem::vKind::vStr, new vstr());
        }
        return end;
    }

    heap::vItem *heap::createVal() {
        if (start == nullptr) {
            start = end = new vItem();
            start->kind = heap::vItem::vKind::vVal;
            start->val = new value();
        } else {
            end = new vItem(end, heap::vItem::vKind::vVal, new value());
        }
        return end;
    }

    heap::vItem *heap::createVec() {
        if (start == nullptr) {
            start = end = new vItem();
            start->kind = heap::vItem::vKind::vVec;
            start->val = new vec<value>();
        } else {
            end = new vItem(end, heap::vItem::vKind::vVec, new vec<value>());
        }
        return end;
    }

    void heap::remove(heap::vItem * v) {
        if (v == start) {
            start = start->next;
            delete v;
        } else {
            vItem *l = v->prev;
            l->next = v->next;
            delete v;
        }
    }

    heap::vItem *heap::createStr(const vstr &v) {
        if (start == nullptr) {
            start = end = new vItem();
            start->kind = heap::vItem::vKind::vStr;
            start->val = new vstr{v};
        } else {
            end = new vItem(end, heap::vItem::vKind::vStr, new vstr{v});
        }
        return end;
    }

    heap::heap() : start(), end() {

    }
}