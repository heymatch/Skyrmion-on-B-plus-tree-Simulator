/**
 *
 *
 **/

#ifndef NODE_H
#define NODE_H

#include "Options.hpp"
#include "KeyPtrSet.hpp"
#include <ostream>

struct Node{
    Node(Options options) : _options(options){
        // _data = new KeyPtrSet[_options.word_length];
        _data = new unsigned[_options.word_length]();
        _bitmap = new bool[_options.word_length]();
    }

    void readData(unsigned idx){
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
    }

    void searchData(unsigned data){
        switch (_options.search_mode)
        {
        case Options::search_function::SEQUENTIAL:
            /* code */
            break;
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

    void updateData(unsigned data){
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

    void insertData(unsigned data){
        /* read node(bitmap and data) */
        /* search insert position */
        switch (_options.insert_mode)
        {
        case Options::insert_function::SEQUENTIAL:
            switch (_options.node_ordering)
            {
            case Options::ordering::SORTED:
                /* code */
                break;
            case Options::ordering::UNSORTED:
            {
                for(int i = 0; i < _options.track_length; ++i){
                    if(!_bitmap[i]){
                        _bitmap[i] = true;
                        _data[i] = data;
                        return;
                    }
                }
                throw "full";
                break;
            }
            default:
                throw "undefined operation";
                break;
            }
            break;
        case Options::insert_function::BIT_BINARY_INSERT:
            /* code */
            throw "Developing";
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    void deleteData(unsigned idx){
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

    // KeyPtrSet *_data;
    unsigned *_data;
    bool *_bitmap;
    ////
    const Options _options;
};

std::ostream &operator<<(std::ostream &out, const Node &right){
    out << "[";
    bool first = true;
    for(int i = 0; i < right._options.track_length; ++i){
        if(first)first = false;
        else out << ", ";
        out << right._data[i];
    }
    out << "]";
    return out;
}

#endif