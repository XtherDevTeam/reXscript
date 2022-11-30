//
// Created by XIaokang00010 on 2022/11/27.
//

#include "value.hpp"
#include "share/share.hpp"

rex::value::vValue::vValue() : vPtr(nullptr) { 

}

rex::value::vValue::vValue(rex::vint v) : vInt(v) { 

}

rex::value::vValue::vValue(rex::vdeci v) : vDeci(v) { 

}

rex::value::vValue::vValue(rex::vbool v) : vBool(v) { 

}

const rex::vint &rex::value::getInt(){ 
    return val.vInt;
}

const rex::vdeci &rex::value::getDeci(){ 
    return val.vDeci;
}

const rex::vbool &rex::value::getBool(){ 
    return val.vBool;
}

rex::vint &rex::value::getIntRef(){ 
    return val.vInt;
}

rex::vdeci &rex::value::getDeciRef(){ 
    return val.vDeci;
}

rex::vbool &rex::value::getBoolRef(){ 
    return val.vBool;
}

rex::value::value() : kind(rex::value::vKind::vNull), val() {

}

rex::value::value(rex::value::vKind k, const rex::value::vValue &v) : kind(k), val(v) {

}