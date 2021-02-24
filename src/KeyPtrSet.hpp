#ifndef KEYPTRSET_H
#define KEYPTRSET_H

#include "Options.hpp"
#include <ostream>

struct KeyPtrSet{
    KeyPtrSet(Options options) : _options(options){
        ptr = new void *;
        key = new unsigned[_options.kp_length - 1];

        _size = 0;
    }

    void setPtr(void *addr){
        ptr = addr;
    }

    void addKey(unsigned val){
        if(_size == _options.kp_length - 1)
            throw "KeyPtrSet overflow";
        key[_size++] = val;
    }

    unsigned *key;
    void *ptr;
    ////
    unsigned _size;
    Options _options;
};

std::ostream &operator<<(std::ostream &out, const KeyPtrSet& right){
    out << "(";
    out << right.ptr << ", ";
    bool first = true;
    for(int i = 0; i < right._options.kp_length - 1; ++i){
        if(first)first = false;
        else out << ", ";
        out << right.key[i];
    }
    out << ")";
    return out;
}

#endif