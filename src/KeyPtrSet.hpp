#ifndef KEYPTRSET_H
#define KEYPTRSET_H

#include "Options.hpp"
#include <ostream>

struct KeyPtrSet{
    KeyPtrSet(uint64_t capacity = 2, bool dataPtr = true){
        // ptr = new Data('O');
        ptr = nullptr;
        key = new Index[capacity - 1]();
        bitmap = new bool[capacity - 1]();

        _size = 0;
        _capacity = capacity;
        _dataPtr = dataPtr;
    }

    KeyPtrSet(const KeyPtrSet &right){
        key = new Index[right._capacity - 1]();
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
        // if(_dataPtr && ptr != nullptr) delete((Data*)ptr);
        delete[] key;
        delete[] bitmap;
    }

    KeyPtrSet &operator=(const KeyPtrSet &right){
        // if(_dataPtr && ptr != nullptr) delete((Data*)ptr);
        delete[] key;
        delete[] bitmap;

        key = new Index[right._capacity - 1]();
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

    void setKey(Offset offset, Index val){
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

    void delKey(Offset offset){
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

    void addKey(Index val){
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

    Index getKey(Offset offset) const{
        return key[offset];
    }

    bool getBitmap(Offset offset) const{
        return bitmap[offset];
    }

    void *getPtr() const{
        return ptr;
    }

    Size getSize() const{
        return _size;
    }

    Size getKeyCapacity() const{
        return _capacity - 1;
    }

    bool isFull() const{
        return _size == _capacity - 1;
    }

    Index *key;
    void *ptr;
    bool *bitmap;
    ////
    Size _size;
    Size _capacity;
    bool _dataPtr;
};

#include <vector>
#include <algorithm>
namespace System{
    Index getMid(KeyPtrSet *arr, Size len, Index insert, Size capacity = 2){
        Size t_len = 1;
        std::vector<Index> V(t_len);
        V[0] = insert;

        for(int i = 0; i < len; ++i){
            
            for(int j = 0; j < capacity - 1; ++j){
                if(arr[i].getBitmap(j)){
                    ++t_len;
                    V.push_back(arr[i].getKey(j));
                }
            }

            // for(int j = 0; j < arr[0]._capacity-1; ++j){
            //     if(arr[i].getBitmap(j)){
            //         ++t_len;
            //         V.push_back(arr[i].getKey(j));
            //     }
            // }
        }

        std::sort(V.begin(), V.end());

        // for(auto &it: V){
        //     std::clog << it << " ";
        // }
        // std::clog << endl;
        
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

    // if(right._dataPtr && right.ptr != nullptr)
    //     out << *((Data *)right.ptr) << ", ";
    // else 
    out << (Size)right.ptr << ", ";

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