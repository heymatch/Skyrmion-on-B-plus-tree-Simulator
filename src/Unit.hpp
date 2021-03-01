#ifndef UNIT_H
#define UNIT_H

#include "Counter.hpp"
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
        _id = UnitId++;
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
                /* EVALUATION */
                break;
            case Options::insert_function::BIT_BINARY_INSERT:
                /* EVALUATION */
                break;
            default:
                throw "undefined insert operation";
            }

            insertCurrentData(idx, data, offset);
        }
        else{
            switch (_options.insert_mode)
            {
            case Options::insert_function::SEQUENTIAL:
                /* EVALUATION */
                break;
            case Options::insert_function::BIT_BINARY_INSERT:
                /* EVALUATION */
                break;
            default:
                throw "undefined insert operation";
            }

            for(int i = 0; i < _options.unit_size; ++i){
                for(int j = 0; j < _options.track_length; ++j){
                    if(_tracks[i]._bitmap[j] && _tracks[i]._ptr[j] != nullptr && idx < _tracks[i]._index[j]){
                        ((Unit *)_tracks[i]._ptr[j])->insertData(idx, data, offset);
                        return;
                    }
                }
                if(_tracks[i]._side != nullptr){
                    ((Unit *)_tracks[i]._side)->insertData(idx, data, offset);
                    return;
                }
            }
        }
    }

    void insertCurrentData(unsigned idx, unsigned data, unsigned offset){
        if(!isLeaf())
            throw "This function is for Leaf node";

        if(isFull(offset)){
            // std::clog << "<log> test point begin 1" << std::endl;
            KeyPtrSet promote = splitNode(idx);
            if(_isRoot){
                deRoot();
                Unit *newRoot = new Unit(_options);
                newRoot->deLeaf();
                newRoot->insertCurrentPointer(*promote.key, this, 0);
                this->connectParentUnit(newRoot, 0);
                newRoot->_tracks[0].connectSideUnit((Unit *)promote.ptr);
                ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);
                _tracks[offset]._parent->insertData(idx, data, 0);
                return;
            }
            //std::clog << "<log> this->_tracks[0]._parent: " << this->_tracks[0]._parent << std::endl;
            this->_tracks[0]._parent->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
            ((Unit *)promote.ptr)->connectParentUnit(this->_tracks[0]._parent, 0);
            _tracks[offset]._parent->insertData(idx, data, 0);
            return;
        }

        _tracks[offset].insertData(idx, data);
    }

    void insertCurrentPointer(unsigned idx, Unit *unit, unsigned offset){
        if(isLeaf())
            throw "This function is for Internal node";
        if(isFull(offset)){
            //std::clog << "<log> test point begin 2" << std::endl;
            KeyPtrSet promote = splitNode(idx);
            if(_isRoot){
                deRoot();
                Unit *newRoot = new Unit(_options);
                newRoot->deLeaf();
                newRoot->insertCurrentPointer(*promote.key, this, 0);
                this->connectParentUnit(newRoot, 0);
                newRoot->_tracks[0].connectSideUnit((Unit *)promote.ptr);
                ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);
                insertCurrentPointer(idx, unit, 0);
                //std::clog << "<log> newRoot: " << (*newRoot)._tracks[0] << std::endl;
                return;
            }
            insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
            ((Unit *)promote.ptr)->connectParentUnit(this, 0);
            return;
        }

        _tracks[offset].connectUnit(idx, unit);
    }

    void connectParentUnit(Unit *unit, unsigned offset){
        _tracks[offset].connectParentNode(unit);
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
                if(!isLeaf()) newUnit->deLeaf();

                unsigned indexSize = _options.track_length;
                unsigned *index = readData(0); // offset

                unsigned insertSize = 1;
                unsigned *insert = new unsigned[insertSize];
                insert[0] = wait_insert_idx;
                unsigned deleteSize = 0;
                
                unsigned *sorted = System::makeSortedArray(index, indexSize, insert, insertSize);
                unsigned *deleteIndexes = System::makeSplitDeleteIndexesArray(deleteSize, index, indexSize, sorted[(indexSize + insertSize) / 2]);

                /*
                switch(_options.node_ordering){
                    case Options::ordering::SORTED:
                        throw "Developing";
                    case Options::ordering::UNSORTED:
                    
                        break;
                    default:
                        throw "undefined split operation";
                }*/
                
                // Use copyHalfNode() to replace below
                if(isLeaf()){
                    for(int i = (indexSize + insertSize) / 2; i < (indexSize + insertSize); ++i){
                        if(sorted[i] != wait_insert_idx)
                            newUnit->insertCurrentData(sorted[i], 0, 0);
                    }
                    _tracks[0].deleteMark(deleteIndexes, deleteSize); // offset
                }
                else{
                    for(int i = (indexSize + insertSize) / 2 + 1; i < (indexSize + insertSize); ++i){
                        if(sorted[i] != wait_insert_idx){
                            newUnit->insertCurrentPointer(sorted[i], (Unit *)_tracks[0]._ptr[i-1], 0);
                            ((Unit *)_tracks[0]._ptr[i-1])->connectParentUnit(newUnit, 0);
                        }
                    }
                    newUnit->_tracks[0].connectSideUnit(_tracks[0]._side);
                    _tracks[0]._side->connectParentUnit(newUnit, 0);
                    newUnit->_tracks[0]._sideBitmap = true;

                    _tracks[0].deleteMark(deleteIndexes, deleteSize); // offset
                    _tracks[0].connectSideUnit((Unit *)_tracks[0]._ptr[(indexSize + insertSize) / 2 - 1]);
                    _tracks[0]._sideBitmap = true;
                }

                promote.setPtr(newUnit);
                promote.addKey(sorted[(indexSize + insertSize) / 2]);

                /* delete allocation memory */
                break;
            }
        case Options::split_merge_function::UNIT:
            /* code */
            throw "Developing merge unit";
        default:
            throw "undefined operation";
        }

        return promote;
    }

    void copyHalfNode(){

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
            if(!_tracks[offset]._bitmap[i]){
                return false;
            }
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
    unsigned _id;
};

std::ostream &operator<<(std::ostream &out, const Unit &right){
    std::clog << "<log> Unit " << right._id << " Print" << std::endl;
    out << "\t[\t";
    /// status
    if(right.isLeaf()) out << "Leaf Unit";
    else out << "Internal Unit";
    out << " " << right._id;
    if(right._isRoot) out << " Root";
    ///
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
                if(right._tracks[i]._bitmap[j] && right._tracks[i]._ptr[j] != nullptr){
                    out << *(Unit *)right._tracks[i]._ptr[j];
                }
            }
            if(right._tracks[i]._sideBitmap && right._tracks[i]._side != nullptr){
                out << *(Unit *)right._tracks[0]._side;
            }
        }
    }
    return out;
}

#endif