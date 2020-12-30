#ifndef KEYPTRSET_H
#define KEYPTRSET_H

#include "Unit.hpp"

struct KeyPtrSet{
    KeyPtrSet(unsigned capacity = 0) : _capacity(capacity){
        _size = 0;
        key = new unsigned(0);
        ptr = (void **)(new Unit*[_capacity]);
    }

    unsigned *key;
    void **ptr;
    ////
    const unsigned _capacity;
    unsigned _size;
};

std::ostream &operator<<(std::ostream &out, const KeyPtrSet& right){
    out << "(";
    out << right.key << ", ";
    bool first = true;
    for(int i = 0; right._capacity; ++i){
        if(first)first = false;
        else out << ", ";
        out << right.ptr[i] << ", ";
    }
    return out;
}

#endif