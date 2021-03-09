#ifndef KEYPTRSET_H
#define KEYPTRSET_H

#include "Options.hpp"
#include <ostream>

struct KeyPtrSet{
    KeyPtrSet(unsigned capacity = 2, bool dataPtr = true){
        ptr = new void *();
        key = new unsigned[capacity - 1]();

        _size = 0;
        _capacity = capacity;
        _dataPtr = dataPtr;
    }

    void setPtr(void *addr){
        ptr = addr;
    }

    void setKey(unsigned offset, unsigned val){
        key[offset] = val;
    }

    void addKey(unsigned val){
        if(_size == _capacity - 1)
            throw "KeyPtrSet overflow";
        key[_size++] = val;
    }

    unsigned getKey(unsigned idx) const{
        return key[idx];
    }

    void *getPtr() const{
        return ptr;
    }

    unsigned *key;
    void *ptr;
    ////
    unsigned _size;
    unsigned _capacity;
    bool _dataPtr;
};

std::ostream &operator<<(std::ostream &out, const KeyPtrSet& right){
    out << "<";
    if(right._dataPtr)
        out << *((unsigned *)right.ptr) << ", ";
    else out << right.ptr << ", ";
    bool first = true;
    for(int i = 0; i < right._capacity - 1; ++i){
        if(first)first = false;
        else out << ", ";
        out << right.key[i];
    }
    out << ">";
    return out;
}

#endif