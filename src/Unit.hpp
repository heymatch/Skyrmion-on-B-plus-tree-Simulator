#ifndef UNIT_H
#define UNIT_H

#include "Options.hpp"
#include "Node.hpp"
#include <ostream>

struct Unit{
    Unit(Options options) : _options(options){
        _tracks = new Node[_options.track_length];
        for(int i = 0; i < _options.track_length; ++i){
            _tracks[i] = Node(_options);
        }
    }

    ~Unit(){
        delete[] _tracks;
    }

    void readData(){
        switch (_options.read_mode)
        {
        case Options::read_function::SEQUENTIAL:
            /* code */
            break;
        case Options::read_function::RANGE_READ:
            /* code */
            break;
        default:
            break;
        }
    }

    void searchData(){
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

    void updateData(){
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

    void insertData(unsigned data, unsigned offset){
        switch (_options.insert_mode)
        {
        case Options::insert_function::SEQUENTIAL:
            _tracks[offset].insertData(data);
            break;
        case Options::insert_function::BIT_BINARY_INSERT:
            /* code */
            throw "Developing";
            break;
        default:
            throw "undefined insert operation";
            break;
        }
    }

    void deleteData(){
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

    void splitNode(){
        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
            /* code */
            break;
        case Options::split_merge_function::UNIT:
            /* code */
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    void mergeNode(){
        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
            /* code */
            break;
        case Options::split_merge_function::UNIT:
            /* code */
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    Node *_tracks;
    ////
    Options _options;
};

std::ostream &operator<<(std::ostream &out, const Unit &right){
    out << "\t[\n";
    bool first = true;
    for(int i = 0; i < right._options.unit_size; ++i){
        if(first)first = false;
        else out << ", \n";
        out << "\t\t" << right._tracks[i];
    }
    out << "\n\t]\n";
    return out;
}

#endif