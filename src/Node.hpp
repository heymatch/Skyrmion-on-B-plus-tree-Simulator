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
        _used_words = new bool[_options.word_length]();
    }

    void readData(unsigned idx){
       switch (_options.read_mode)
       {
       case Options::function_list::TRD_READ:
           /* code */
           break;
       
       default:
           throw "undefined operation";
           break;
       } 
    }

    void updateData(unsigned data){
        switch (_options.update_mode)
        {
        case Options::function_list::TRD_UPDATE:
            /* code */
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    void insertData(unsigned data){
        switch (_options.insert_mode)
        {
        case Options::function_list::TRD_INSERT:
            for(int i = 0; i < _options.track_length; ++i){
                if(!_used_words[i]){
                    _used_words[i] = true;
                    _data[i] = data;
                    break;
                }
            }
            throw "full";
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    void deleteData(unsigned idx){
        switch (_options.delete_mode)
        {
        case Options::function_list::TRD_DELETE:
            /* code */
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    // KeyPtrSet *_data;
    unsigned *_data;
    bool *_used_words;
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