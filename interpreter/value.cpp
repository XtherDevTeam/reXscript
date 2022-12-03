//
// Created by XIaokang00010 on 2022/11/27.
//

#include "value.hpp"
#include "share/share.hpp"

namespace rex {
    value::vValue::vValue() : vPtr(nullptr) {

    }

    value::vValue::vValue(vint v) : vInt(v) {

    }

    value::vValue::vValue(vdeci v) : vDeci(v) {

    }

    value::vValue::vValue(vbool v) : vBool(v) {

    }

    const vint &value::getInt(){
        return val.vInt;
    }

    const vdeci &value::getDeci(){
        return val.vDeci;
    }

    const vbool &value::getBool(){
        return val.vBool;
    }

    vint &value::getIntRef(){
        return val.vInt;
    }

    vdeci &value::getDeciRef(){
        return val.vDeci;
    }

    vbool &value::getBoolRef(){
        return val.vBool;
    }

    value::value() : kind(value::vKind::vNull), val() {

    }

    value::value(value::vKind k, const value::vValue &v) : kind(k), val(v) {
    }

    bool value::deleteMember(const vstr &l) {
        if (auto it = object.find(l); it != object.end()) {
            object.erase(it);
            return true;
        } else {
            return false;
        }
    }

    bool value::setMember(const vstr &l, const value &r) {
        object[l] = new value{r};
    }

    value::~value() {
        for (auto &i : object)
            delete i.second;
        object.clear();
    }

    value &value::operator[](const vstr &l) {
        if (auto it = object.find(l); it != object.end())
            return *it->second;
        return *(object[l] = new value{});
    }

    value::value(const value &v) {
        for (auto &i : v.object) {
            object[i.first] = new value{*i.second}; // deep copy from source
        }
    }
}