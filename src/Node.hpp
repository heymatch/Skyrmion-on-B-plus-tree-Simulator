/**
 *
 *
 **/

#ifndef NODE_H
#define NODE_H

#include "Options.hpp"
#include <ostream>

struct Node{
    Node(Options options = Options(), bool isLeaf = true) : _options(options){
        //_data = new KeyPtrSet[_options.word_length];
        _index = new unsigned[_options.word_length]();
        _bitmap = new bool[_options.word_length]();
        _ptr = new void*[_options.word_length]();
        _isLeaf = isLeaf;
    }

    // Return array of indexes
    // 0 index
    unsigned *readData(unsigned lower, unsigned upper){
        if(upper < lower)
            throw "invalid read bound";
         
        // calculate performance
        switch (_options.read_mode)
        {
        case Options::read_function::SEQUENTIAL:
            /* code */
            break;
        case Options::read_function::RANGE_READ:
            /* code */
            break;
        default:
            throw "undefined read operation";
            break;
        }

        unsigned *arr = new unsigned[upper - lower + 1];
        for(int i = lower; i < upper; ++i){
            arr[i] = _index[i];
        }
        return arr; 
    }

    // Return data pointer.
    unsigned *searchData(unsigned idx){
        switch (_options.search_mode)
        {
        case Options::search_function::SEQUENTIAL:
            for(int i = 0; i < _options.track_length; ++i){
                if(_bitmap[i] && _index[i] == idx){
                    return (unsigned *)_ptr[i];
                }
            }
            throw "search not found";
        case Options::search_function::TRAD_BINARY_SEARCH:
            /* code */
            break;
        case Options::search_function::BIT_BINARY_SEARCH:
            /* code */
            break;
        default:
            throw "undefined search operation";
            break;
        }
    }

    void updateData(unsigned index, unsigned data){
        switch (_options.update_mode)
        {
        case Options::update_function::OVERWRITE:
            /* code */
            break;
        case Options::update_function::PERMUTATION_WRITE:
            /* code */
            break;
        case Options::update_function::PERMUTE_WORD_COUNTER:
            /* code */
            break;
        case Options::update_function::PERMUTE_FEW_COUNTER:
            /* code */
            break;
        default:
            throw "undefined update operation";
            break;
        }
    }

    // insertDate() and connectNode() can be merge to one function after KeyPtrSet Complete

    // For leaf node, ptr points to data
    void insertData(unsigned idx, unsigned data){
        /* read node(metadata, bitmap and data) */
        unsigned insertPos = getInsertPosition(); /* EVALUATE SEARCH */
        _bitmap[insertPos] = true;
        _index[insertPos] = idx;
        /* EVALUATE INSERTION */
        _ptr[insertPos] = new unsigned(data);
    }

    // For internal node, ptr points to next node
    void connectNode(unsigned idx, Unit *unit){
        /* read node(metadata, bitmap and data) */
        unsigned insertPos = getInsertPosition(); /* EVALUATE SEARCH */
        _bitmap[insertPos] = true;
        _index[insertPos] = idx;
        /* EVALUATE INSERTION */
        _ptr[insertPos] = unit;
    }

    void deleteData(unsigned data){
        switch (_options.delete_mode)
        {
        case Options::delete_function::SEQUENTIAL:
            /* code */
            break;
        case Options::delete_function::BALANCE:
            /* code */
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    unsigned getInsertPosition(){
        switch (_options.insert_mode)
        {
        case Options::insert_function::SEQUENTIAL:
            switch (_options.node_ordering)
            {
            case Options::ordering::SORTED:
                throw "Developing";
            case Options::ordering::UNSORTED:
                for(int i = 0; i < _options.track_length; ++i){
                    if(!_bitmap[i]){
                        return i;
                    }
                }
                throw "full";
            default:
                throw "undefined ordering";
            }
            break;
        case Options::insert_function::BIT_BINARY_INSERT:
            /* code */
            throw "Developing";
        default:
            throw "undefined insert operation";
        }
    }

    // KeyPtrSet *_data;
    unsigned *_index;
    void **_ptr;
    bool *_bitmap;
    bool _isLeaf;
    //bool _isValid;
    ////
    Options _options;
};

std::ostream &operator<<(std::ostream &out, const Node &right){
    out << "(";
    bool first = true;
    for(int i = 0; i < right._options.track_length; ++i){
        if(first)first = false;
        else out << ", ";
        out << right._index[i];
    }
    out << ")";
    return out;
}

#endif