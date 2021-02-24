#ifndef UNIT_H
#define UNIT_H

#include "System.hpp"
#include "Options.hpp"
#include "Node.hpp"
#include "KeyPtrSet.hpp"
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

    Unit *insertData(unsigned idx, unsigned data, unsigned offset){
        if(isLeaf()){
            switch (_options.insert_mode)
            {
            case Options::insert_function::SEQUENTIAL:
                return insertCurrentData(idx, data, offset);
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

        return this;
    }

    Unit *insertCurrentData(unsigned idx, unsigned data, unsigned offset){
        if(isFull(offset)){
            KeyPtrSet promote = splitNode(idx);
            if(_isRoot){
                deRoot();
                Unit *newRoot = new Unit(_options);
                newRoot->deLeaf();
                newRoot->insertCurrentPointer(*promote.key, this, 0);
                newRoot->_tracks[0].connectSideUnit((Unit *)promote.ptr);
                return newRoot;
            }
        }

        _tracks[offset].insertData(idx, data);

        return this;
    }

    Unit *insertCurrentPointer(unsigned idx, Unit *unit, unsigned offset){
        if(isFull(offset)){
            splitNode(idx);
        }

        _tracks[offset].connectUnit(idx, unit);

        return this;
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
    KeyPtrSet splitNode(unsigned wait_insert_idx){
        KeyPtrSet promote(_options);

        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
            {
                Unit *newUnit = new Unit(_options);
                newUnit->deRoot();

                unsigned readSize = _options.track_length;
                unsigned *read = readData(0); // offset
                unsigned insertSize = 1;
                unsigned *insert = new unsigned[insertSize];
                insert[0] = wait_insert_idx;
                
                unsigned *sorted = System::makeSortedArray(read, readSize, insert, insertSize);
                unsigned *deleteIndexes = System::makeSplitDeleteIndexesArray(read, readSize, sorted[(readSize + insertSize) / 2]);

                switch(_options.node_ordering){
                    case Options::ordering::SORTED:
                        throw "Developing";
                    case Options::ordering::UNSORTED:
                    {
                        for(int i = (readSize + insertSize) / 2; i < (readSize + insertSize); ++i){
                            newUnit->insertCurrentData(sorted[i], 0, 0);
                        }
                        _tracks[0].deleteMark(deleteIndexes, readSize / 2); // offset
                    }
                        break;
                    default:
                        throw "undefined split operation";
                }

                promote.setPtr(newUnit);
                promote.addKey(sorted[(readSize + insertSize) / 2]);

                /* delete allocation memory */
                break;
            }
        case Options::split_merge_function::UNIT:
            /* code */
            break;
        default:
            throw "undefined operation";
        }

        return promote;
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

    bool isLeaf() const{
        return _tracks[0]._isLeaf;
    }

    void deLeaf(){
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i]._isLeaf = false;
        }
    }

    Node *_tracks;
    ////

    void deRoot(){
        _isRoot = false;
    }

    bool _isRoot;
    Options _options;
};

std::ostream &operator<<(std::ostream &out, const Unit &right){
    out << "\t[\t";
    if(right.isLeaf()){
        out << "Leaf Unit";
    }
    else{
        out << "Internal Unit";
    }
    out << ":\n";
    bool first = true;
    for(int i = 0; i < right._options.unit_size; ++i){
        if(first)first = false;
        else out << ", \n";
        out << "\t\t" << right._tracks[i];
    }
    out << "\n\t]\n";
    if(!right.isLeaf()){
        for(int i = 0; i < right._options.unit_size; ++i){
            for(int j = 0; j < right._options.track_length; ++j){
                if(right._tracks[i]._ptr[j] != nullptr){
                    out << *(Unit *)right._tracks[i]._ptr[j];
                }
            }
            if(right._tracks[i]._side != nullptr){
                out << *(Unit *)right._tracks[0]._side;
            }
        }
    }
    return out;
}

#endif