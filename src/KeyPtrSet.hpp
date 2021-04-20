#ifndef KEYPTRSET_H
#define KEYPTRSET_H

#define uint64_t uint64_t

#include "Options.hpp"
#include <ostream>

struct KeyPtrSet{
    KeyPtrSet(uint64_t capacity = 2, bool dataPtr = true){
        ptr = new Data('O');
        key = new uint64_t[capacity - 1]();
        bitmap = new bool[capacity - 1]();

        _size = 0;
        _capacity = capacity;
        _dataPtr = dataPtr;
    }

    KeyPtrSet(const KeyPtrSet &right){
        key = new uint64_t[right._capacity - 1]();
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

    ~KeyPtrSet(){
        delete[] key;
        delete[] bitmap;
    }

    KeyPtrSet &operator=(const KeyPtrSet &right){
        delete[] key;
        delete[] bitmap;

        key = new uint64_t[right._capacity - 1]();
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

    void setKey(uint64_t offset, uint64_t val){
        key[offset] = val;
        if(!bitmap[offset])
            ++_size;
        bitmap[offset] = true;

        //! only for two keys
        if(_capacity == 3){
            if(bitmap[0] && bitmap[1] && key[0] > key[1]){
                swap(key[0], key[1]);
            }
        }
    }

    void delKey(uint64_t offset){
        if(bitmap[offset])
            --_size;
        bitmap[offset] = false;

        //! only for two keys
        if(_capacity == 3){
            if(!bitmap[0] && bitmap[1]){
                swap(key[0], key[1]);
                swap(bitmap[0], bitmap[1]);
            }
        }
    }

    void delAll(){
        for(int i = 0; i < _capacity - 1; ++i){
            bitmap[i] = false;
        }
        _size = 0;
    }

    void addKey(uint64_t val){
        if(_size == _capacity - 1)
            throw "KeyPtrSet overflow";
        
        bitmap[_size] = true;
        key[_size] = val;
        ++_size;

        //! only for two keys
        if(_capacity == 3){
            if(bitmap[0] && bitmap[1] && key[0] > key[1]){
                swap(key[0], key[1]);
            }
        }
    }

    uint64_t getKey(uint64_t offset) const{
        return key[offset];
    }

    bool getBitmap(uint64_t offset) const{
        return bitmap[offset];
    }

    void *getPtr() const{
        return ptr;
    }

    uint64_t getSize() const{
        return _size;
    }

    bool isFull() const{
        return _size == _capacity - 1;
    }

    uint64_t *key;
    void *ptr;
    bool *bitmap;
    ////
    uint64_t _size;
    uint64_t _capacity;
    bool _dataPtr;
};

#include <vector>
#include <algorithm>
namespace System{
    uint64_t getMid(KeyPtrSet *arr, uint64_t len, uint64_t insert, Size capacity = 2){
        uint64_t t_len = 1;
        std::vector<uint64_t> V(t_len);
        V[0] = insert;

        for(int i = 0; i < len; ++i){
            
            for(int j = 0; j < capacity - 1; ++j){
                if(arr[i].getBitmap(j)){
                    ++t_len;
                    V.push_back(arr[i].getKey(j));
                }
            }
            
            // if(arr[i].getBitmap(0)){
            //     ++t_len;
            //     V.push_back(arr[i].getKey(0));
            // }
        }

        std::sort(V.begin(), V.end());

        /*
        for(auto &it: V){
            std::clog << it << " ";
        }
        std::clog << endl;
        */
        
        if(t_len % 2 == 0){
            //std::clog << "<log> V[t_len / 2 - 1]: " << V[t_len / 2 - 1] << std::endl;
            return V[t_len / 2];
        }
        else{
            //std::clog << "<log> V[t_len / 2]: " << V[t_len / 2] << std::endl;
            return V[t_len / 2];
        }
        
    }
}

std::ostream &operator<<(std::ostream &out, const KeyPtrSet& right){
    out << "<";

    if(right._dataPtr)
        out << *((Data *)right.ptr) << ", ";
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