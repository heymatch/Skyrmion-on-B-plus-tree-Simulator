#ifndef UNIT_H
#define UNIT_H

#include "Counter.hpp"
#include "System.hpp"
#include "Options.hpp"
#include "KeyPtrSet.hpp"
#include "Node.hpp"
#include <ostream>

struct Unit{
    Unit(Options options) : _options(options){
        _tracks = new Node[_options.unit_size];
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i] = Node(_options, false);
        }
        _isRoot = true;
        _id = UnitId++;
    }

    ~Unit(){
        delete[] _tracks;
    }

    KeyPtrSet *readData(unsigned offset){
        return _tracks[offset].readData(0, _options.track_length);
    }

    // Return data pointer
    unsigned *searchData(unsigned idx){
        unsigned *dataPtr = nullptr;
        if(isLeaf()){
            switch (_options.search_mode){
            case Options::search_function::SEQUENTIAL:
                for(int i = 0; i < _options.unit_size; ++i){
                    try{
                        dataPtr = (unsigned *)_tracks[i].searchData(idx);
                    }
                    catch(int e){
                        if(e != -1)
                            throw "search error";
                    }
                }
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
        else{
            Unit *nextPtr = nullptr;
            for(int i = 0; i < _options.unit_size; ++i){
                if(nextPtr != nullptr)break;
                try{
                    nextPtr = (Unit *)_tracks[i].searchData(idx);
                }
                catch(int e){
                    if(e != -1)
                        throw "search error";
                }
            }
            dataPtr = nextPtr->searchData(idx);
        }
        
        return dataPtr;
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
                    if(_tracks[i]._bitmap[j] && _tracks[i]._data[j].getPtr() != nullptr && idx < _tracks[i]._data[j].getKey(0)){
                        ((Unit *)_tracks[i]._data[j].getPtr())->insertData(idx, data, offset);
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
            KeyPtrSet promote = splitNode(idx, offset);
            if(_isRoot){
                deRoot();

                Unit *newRoot = new Unit(_options);
                newRoot->deLeaf();

                newRoot->insertCurrentPointer(*promote.key, this, 0);
                this->connectParentUnit(newRoot);

                newRoot->connectSideUnit((Unit *)promote.ptr);
                ((Unit *)promote.ptr)->connectParentUnit(newRoot);

                connectSideUnit((Unit *)promote.ptr);
                if(idx < promote.getKey(0))
                    insertCurrentData(idx, data, offset);
                else
                    getSideUnit()->insertCurrentData(idx, data, 0);

                return;
            }
            //std::clog << "<log> this->_tracks[0]._parent: " << this->_tracks[0]._parent << std::endl;
            this->_tracks[0]._parent->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
            if(((Unit *)promote.ptr)->getParentUnit() == nullptr)
                ((Unit *)promote.ptr)->connectParentUnit(getParentUnit());

            _tracks[offset]._parent->insertData(idx, data, 0);
            return;
        }

        _tracks[offset].insertData(idx, new unsigned(data));
    }

    void insertCurrentPointer(unsigned idx, Unit *unit, unsigned offset){
        if(isLeaf())
            throw "This function is for Internal node";
        if(isFull(offset)){
            //std::clog << "<log> test point begin 2" << std::endl;
            KeyPtrSet promote = splitNode(idx, offset);
            if(_isRoot){
                deRoot();

                Unit *newRoot = new Unit(_options);
                newRoot->deLeaf();

                newRoot->insertCurrentPointer(*promote.key, this, 0);
                this->connectParentUnit(newRoot);

                newRoot->connectSideUnit((Unit *)promote.ptr);
                ((Unit *)promote.ptr)->connectParentUnit(newRoot);
                
                if(idx < promote.getKey(0))
                    insertCurrentPointer(idx, unit, offset);
                else if(idx > promote.getKey(0))
                    getParentUnit()->getSideUnit()->insertCurrentPointer(idx, unit, 0);
                else 
                    getParentUnit()->getSideUnit()->_tracks[0]._data[0].setPtr(unit);
                //std::clog << "<log> newRoot: " << (*newRoot)._tracks[0] << std::endl;
                return;
            }
            //insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
            //((Unit *)promote.ptr)->connectParentUnit(this, 0);
            this->_tracks[0]._parent->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
            if(((Unit *)promote.ptr)->getParentUnit() == nullptr)
                ((Unit *)promote.ptr)->connectParentUnit(getParentUnit());


            Unit *rightUnit = getParentRightUnit(idx);
            if(idx < promote.getKey(0))
                insertCurrentPointer(idx, unit, offset);
            else if(idx > promote.getKey(0))
                rightUnit->insertCurrentPointer(idx, unit, 0);
            else{
                rightUnit->_tracks[0]._data[0].setPtr(unit);
                ((Unit *)rightUnit->_tracks[0]._data[0].getPtr())->connectParentUnit(rightUnit);
                //std::clog << "<log> rightUnit->_tracks[0]._data[0].getPtr(): " << rightUnit->_tracks[0]._data[0].getPtr() << std::endl;
                //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
                //((Unit *)promote.getPtr())->connectParentUnit(getParentRightUnit());
            }
                

            return;
        }

        _tracks[offset].insertData(idx, unit);
    }

    void connectSideUnit(Unit *unit){
        _tracks[0].connectSideUnit(unit);
    }

    Unit *getSideUnit() const{
        return _tracks[0]._side;
    }

    void connectParentUnit(Unit *unit){
        _tracks[0].connectParentNode(unit);
    }

    Unit *getParentUnit() const{
        return _tracks[0]._parent;
    }

    Unit *getParentRightUnit(unsigned idx) const{
        for(int i = 0; i < _options.track_length-1; ++i){
            if(getParentUnit()->_tracks[0]._bitmap[i+1] && idx < getParentUnit()->_tracks[0]._data[i+1].getKey(0)){
                return (Unit *)_tracks[0]._parent->_tracks[0]._data[i+1].getPtr();
            }
        }
        return _tracks[0]._parent->getSideUnit();
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
    KeyPtrSet splitNode(unsigned wait_insert_idx, unsigned offset){
        KeyPtrSet promote(2);

        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
        {
            Unit *newUnit = new Unit(_options);
            newUnit->deRoot();
            if(!isLeaf()) newUnit->deLeaf();

            unsigned promoteKey = System::getMid(_tracks[offset]._data, _options.track_length, wait_insert_idx);
            promote.setPtr(newUnit);
            promote.addKey(promoteKey);
            
            copyHalfNode(_tracks[offset], newUnit->_tracks[0], promote);
            if(!isLeaf()){
                getSideUnit()->connectParentUnit(this);
                newUnit->getSideUnit()->connectParentUnit(newUnit);
            }
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

    void copyHalfNode(Node &source, Node &destination, KeyPtrSet promote = 0){
        if(isLeaf()){
            switch(_options.node_ordering){
                case Options::ordering::SORTED:
                    for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                        destination._data[j] = source._data[i];

                        destination._bitmap[j] = true;
                        source._bitmap[i] = false;
                    }
                    break;
                case Options::ordering::UNSORTED:
                    break;
                default:
                    throw "undefined ordering";
            }
        }
        else{
            
            for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                destination._data[j] = source._data[i];

                destination._bitmap[j] = true;
                source._bitmap[i] = false;

                ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr());
            }
            destination.connectSideUnit(source._side);

            bool promoteMid = true;
            for(int i = 0; i < _options.track_length / 2; ++i){
                if(source._data[i].getKey(0) == promote.getKey(0)){
                    source.deleteMark(i);
                    promoteMid = false;
                    source.connectSideUnit((Unit *)source._data[i].getPtr());
                }
            }
            for(int i = 0; i < _options.track_length / 2; ++i){
                if(destination._data[i].getKey(0) == promote.getKey(0)){
                    destination.deleteMark(i);
                    promoteMid = false;
                    source.connectSideUnit((Unit *)destination._data[0].getPtr());
                }
            }

            if(promoteMid){
                source.connectSideUnit((Unit *)destination._data[0].getPtr());
                //destination._data[0].setPtr((Unit *)promote.getPtr());
            }
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

    bool isFull(unsigned offset) const{
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
/*
    bool isValid(unsigned offset) const{
    }
*/
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
    //std::clog << "<log> Unit " << right._id << " Print" << std::endl;
    out << "\t[\t";
    /// status
    if(right.isLeaf()) out << "Leaf Unit";
    else out << "Internal Unit";
    out << " " << right._id;
    out << " " << &right;
    out << " Parent: " << right.getParentUnit();
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
                if(right._tracks[i]._bitmap[j] && right._tracks[i]._data[j].getPtr() != nullptr){
                    out << *(Unit *)right._tracks[i]._data[j].getPtr();
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