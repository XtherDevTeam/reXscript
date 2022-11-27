//
// Created by XIaokang00010 on 2022/11/27.
//

#include "value.hpp"
#include "share/share.hpp"

rex::value::vValue::vValue(rex::vint v) : vInt(v) { 

}

rex::value::vValue::vValue(rex::vdeci v) : vDeci(v) { 

}

rex::value::vValue::vValue(rex::vbool v) : vBool(v) { 

}

const rex::vint &rex::value::getInt(){ 
    return value.vInt;
}

const rex::vdeci &rex::value::getDeci(){ 
    return value.vDeci;
}

const rex::vbool &rex::value::getBool(){ 
    return value.vBool;
}

rex::vint &rex::value::getIntRef(){ 
    return value.vInt;
}

rex::vdeci &rex::value::getDeciRef(){ 
    return value.vDeci;
}

rex::vbool &rex::value::getBoolRef(){ 
    return value.vBool;
}