#ifndef KEYPTRSET_H
#define KEYPTRSET_H

#include "Options.hpp"
#include <ostream>

struct KeyPtrSet{
    KeyPtrSet(unsigned capacity = 2, bool dataPtr = true){
        ptr = new void *();
        key = new unsigned[capacity - 1]();
        bitmap = new bool[capacity - 1]();

        _size = 0;
        _capacity = capacity;
        _dataPtr = dataPtr;
    }

    KeyPtrSet(const KeyPtrSet &right){
        key = new unsigned[right._capacity - 1]();
        bitmap = new bool[right._capacity - 1]();

        _size = right._size;
        _capacity = right._capacity;
        _dataPtr = right._dataPtr;

        ptr = right.ptr;
        for(int i = 0; i < _capacity - 1; ++i){
            key[i] = right.key[i];
            bitmap[i] = right.bitmap[i];
        }
    }

    KeyPtrSet &operator=(const KeyPtrSet &right){
        key = new unsigned[right._capacity - 1]();
        bitmap = new bool[right._capacity - 1]();

        _size = right._size;
        _capacity = right._capacity;
        _dataPtr = right._dataPtr;

        ptr = right.ptr;
        for(int i = 0; i < _capacity - 1; ++i){
            key[i] = right.key[i];
            bitmap[i] = right.bitmap[i];
        }

        return *this;
    }

    void setPtr(void *addr){
        ptr = addr;
    }

    void setKey(unsigned offset, unsigned val){
        key[offset] = val;
        if(!bitmap[offset])
            ++_size;
        bitmap[offset] = true;
    }

    void delKey(unsigned offset){
        if(bitmap[offset])
            --_size;
        bitmap[offset] = false;
    }

    void delAll(){
        for(int i = 0; i < _capacity - 1; ++i){
            bitmap[i] = false;
        }
        _size = 0;
    }

    void addKey(unsigned val){
        if(_size == _capacity - 1)
            throw "KeyPtrSet overflow";
        
        bitmap[_size] = true;
        key[_size] = val;
        ++_size;
    }

    unsigned getKey(unsigned offset) const{
        return key[offset];
    }

    bool getBitmap(unsigned offset) const{
        return bitmap[offset];
    }

    void *getPtr() const{
        return ptr;
    }

    unsigned getSize() const{
        return _size;
    }

    bool isFull() const{
        return _size == _capacity - 1;
    }

    unsigned *key;
    void *ptr;
    bool *bitmap;
    ////
    unsigned _size;
    unsigned _capacity;
    bool _dataPtr;
};

#include <vector>
#include <algorithm>
namespace System{
    unsigned getMid(KeyPtrSet *arr, unsigned len, unsigned insert){
        unsigned t_len = (arr[0]._capacity - 1) * len + 1;
        std::vector<unsigned> V(t_len);

        int it = 0;
        for(int i = 0; i < len; ++i){
            for(int j = 0; j < arr[i]._capacity-1; ++j, ++it){
                V[it] = arr[i].getKey(j);
            }
        }
        V[it++] = insert;

        std::sort(V.begin(), V.end());
        //std::clog << "<log> V[t_len / 2]: " << V[t_len / 2] << std::endl;
        return V[t_len / 2];
    }
}

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
        if(!right.bitmap[i]) out << "*";
    }
    out << ">";
    return out;
}

#endif