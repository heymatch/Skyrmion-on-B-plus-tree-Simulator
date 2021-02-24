#ifndef UNIT_H
#define UNIT_H

#include "Options.hpp"
#include "Node.hpp"
//#include "KeyPtrSet.hpp"
#include <ostream>

struct Unit{
    Unit(Options options) : _options(options){
        _tracks = new Node[_options.track_length];
        for(int i = 0; i < _options.track_length; ++i){
            _tracks[i] = Node(_options);
        }
        _isRoot = true;
    }

    ~Unit(){
        delete[] _tracks;
    }

    unsigned *readData(unsigned offset){
        return _tracks[offset].readData(0, _options.track_length);
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

    void insertData(unsigned idx, unsigned data, unsigned offset){
        if(isLeaf()){
            switch (_options.insert_mode)
            {
            case Options::insert_function::SEQUENTIAL:
                insertCurrent(idx, data, offset);
                break;
            case Options::insert_function::BIT_BINARY_INSERT:
                /* code */
                throw "Developing";
                break;
            default:
                throw "undefined insert operation";
            }
        }
        else{
            throw "Developing";
        }
    }

    void insertCurrent(unsigned idx, unsigned data, unsigned offset){
        if(isFull(offset)){
            splitNode(idx);

        }
        _tracks[offset].insertData(idx, data);
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

    // Return KeyPtrSet
    unsigned splitNode(unsigned wait_insert_idx){
        
        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
            {
                unsigned readSize = _options.track_length;
                unsigned *read = readData(0);
                unsigned insertSize = 1;
                unsigned *insert = new unsigned[insertSize];
                insert[0] = wait_insert_idx;
                
                unsigned *sorted = makeSortedArray(read, readSize, insert, insertSize);
                switch(_options.node_ordering){
                    case Options::ordering::SORTED:
                        throw "Developing";
                    case Options::ordering::UNSORTED:
                    {
                        Unit *newUnit = new Unit(_options);
                        for(int i = (readSize + insertSize) / 2; i < (readSize + insertSize); ++i){
                            newUnit->insertCurrent(sorted[i], 0);
                        }
                    }
                        break;
                    default:
                        throw "undefined split operation";
                }
                break;
            }
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

    bool isFull(unsigned offset){
        for(int i = 0; i < _options.track_length; ++i){
            if(!_tracks[offset]._bitmap[i])
                return false;
        }
        return true;
    }

    bool isLeaf(){
        return _tracks[0]._isLeaf;
    }

    void deLeaf(){
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i]._isLeaf = false;
        }
    }

    Node *_tracks;
    ////
    bool _isRoot;
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