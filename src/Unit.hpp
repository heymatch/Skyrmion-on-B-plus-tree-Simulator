#ifndef UNIT_H
#define UNIT_H

#include <ostream>

#include "Options.hpp"
#include "KeyPtrSet.hpp"
#include "Node.hpp"
#include "Counter.hpp"

namespace System{
    Unit *allocUnit(Options options, bool isLeaf = true);
}

struct Unit{
    Unit(Options options, bool isLeaf = true) : _options(options){
        // init tracks
        _tracks = new Node[_options.unit_size]();
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i] = Node(_options, false, isLeaf);
            //std::clog << "<log> _tracks[i]: " << _tracks[i] << std::endl;
        }
        _tracks[0].setValid(true);

        // assume new unit is root
        _isRoot = true;

        // get unit id
        _id = UnitId++;
    }

    Unit(const Unit &right){
        _options = right._options;

        _tracks = new Node[_options.unit_size]();
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i] = right._tracks[i];
        }

        // assume new unit is root
        _isRoot = right._isRoot;

        // get unit id
        _id = right._id;
    }

    ~Unit(){
        delete[] _tracks;
    }

    Unit &operator=(const Unit &right){
        _options = right._options;

        _tracks = new Node[_options.unit_size]();
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i] = right._tracks[i];
        }

        // assume new unit is root
        _isRoot = right._isRoot;

        // get unit id
        _id = right._id;

        return *this;
    }

    /* Major Functions */

    KeyPtrSet *readData(uint64_t unit_offset){
        return _tracks[unit_offset].readData(0, _options.track_length);
    }

    /**
     * * Return data pointer
     * * DONE
     */
    uint64_t *searchData(uint64_t idx, uint64_t &unit_offset){
        uint64_t *dataPtr = nullptr;

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            if(isLeaf()){
                try{
                    dataPtr = (uint64_t *)_tracks[unit_offset].searchData(idx, unit_offset);
                }
                catch(int e){
                    if(e != -1) throw "search error";
                }
            }
            else{
                try{
                    dataPtr = (uint64_t *)((Unit *)_tracks[unit_offset].searchData(idx, unit_offset))->searchData(idx, unit_offset);
                }
                catch(int e){
                    if(e != -1) throw "search error";
                }
            }
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            if(isLeaf()){
                try{
                    //std::clog << "<log> <searchData()> idx: " << idx << std::endl;
                    //std::clog << "<log> <searchData()> _id: " << _id << std::endl;
                    //std::clog << "<log> <searchData()> unit_offset: " << unit_offset << std::endl;
                    dataPtr = (uint64_t *)_tracks[unit_offset].searchData(idx, unit_offset);
                    /*
                    if(dataPtr == nullptr){
                        if(unit_offset + 1 < _options.unit_size){
                            unit_offset += 1;
                            dataPtr = (uint64_t *)_tracks[unit_offset].searchData(idx, unit_offset);
                        }
                    }
                    if(dataPtr == nullptr){
                        if(hasBackSideUnit()){
                            unit_offset = 0;
                            dataPtr = getBackSideUnit()->searchData(idx, unit_offset);
                        }
                    }*/
                }
                catch(int e){
                    if(e != -1) throw "search error";
                }
            }
            else{
                try{
                    // std::clog << "<log> <searchData()> idx: " << idx << std::endl;
                    // std::clog << "<log> <searchData()> _id: " << _id << std::endl;
                    // std::clog << "<log> <searchData()> unit_offset: " << unit_offset << std::endl;
                    Unit *nextUnit = (Unit *)_tracks[unit_offset].searchData(idx, unit_offset);
                    if(unit_offset == _options.unit_size){
                        if(nextUnit->_tracks[1].isValid()){
                            unit_offset = 1;
                        }
                        else{
                            unit_offset = 0;
                        }
                    }
                    dataPtr = (uint64_t *)nextUnit->searchData(idx, unit_offset);
                }
                catch(int e){
                    if(e != -1) throw "search error";
                }
            }
        }

        
        
        return dataPtr;
    }

    void updateData(uint64_t idx, uint64_t data, uint64_t offset){

    }

    /**
     * * Controller of insertion
     * TODO evaluation
     */
    void insertData(uint64_t idx, uint64_t data, uint64_t unit_offset, uint64_t data_enter_offset){

        if(isLeaf()){
            switch (_options.search_mode){
            case Options::search_function::SEQUENTIAL:
                //TODO evaluation 
                break;
            case Options::search_function::TRAD_BINARY_SEARCH:
            {
                int l = 0;
                int h = 0;
                for(int i = _options.track_length - 1; i >= 0; --i){
                    if(_tracks[unit_offset]._data[i].getBitmap(0)){
                        h = i;
                        break;
                    }
                }
                while (l <= h) {
                    int mid = (l + h) / 2;
                    
                    
                    if (_tracks[unit_offset]._data[mid].getKey(0) > idx) {
                        if(!_tracks[unit_offset]._data[mid].getBitmap(0)){
                            h = mid - 1;
                            continue;
                        }
                        h = mid - 1;
                    }
                    else if (_tracks[unit_offset]._data[mid].getKey(0) < idx) {
                        if(!_tracks[unit_offset]._data[mid].getBitmap(0)){
                            l = mid + 1;
                            continue;
                        }
                        l = mid + 1;                   
                    }
                    else{
                        _tracks[unit_offset]._shiftCounter.count(2 * _options.word_length);
                        _tracks[unit_offset]._readCounter.count(_options.word_length);
                        break;
                    }
                    _tracks[unit_offset]._shiftCounter.count(2 * _options.word_length);
                    _tracks[unit_offset]._readCounter.count(_options.word_length);
                }
            }
                break;
            case Options::search_function::BIT_BINARY_SEARCH:
                _tracks[unit_offset]._shiftCounter.count(2 * _options.word_length);
                _tracks[unit_offset]._readCounter.count(_options.word_length * 2 * _options.track_length);
                break;
            default:
                throw "undefined search operation";
            }

            insertCurrentData(idx, data, unit_offset, data_enter_offset);
        }
        else{
            if(_tracks[unit_offset]._sideBack == nullptr){
                throw "insert side error";
            }
            
            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                //* Evaluation
                switch (_options.search_mode){
                case Options::search_function::SEQUENTIAL:
                    //TODO evaluation 
                    break;
                case Options::search_function::TRAD_BINARY_SEARCH:
                    _tracks[unit_offset]._shiftCounter.count(2 * _options.word_length * Evaluation::log2(_options.track_length));
                    _tracks[unit_offset]._readCounter.count(_options.word_length * 2 * _options.track_length);
                    break;
                case Options::search_function::BIT_BINARY_SEARCH:
                    _tracks[unit_offset]._shiftCounter.count(2 * _options.word_length);
                    _tracks[unit_offset]._readCounter.count(_options.word_length * 2 * _options.track_length);
                    break;
                default:
                    throw "undefined search operation";
                }
                
                for(int i = 0; i < _options.track_length; ++i){
                    if(
                        _tracks[unit_offset]._data[i].getBitmap(0) &&
                        _tracks[unit_offset]._data[i].getPtr() != nullptr &&
                        idx < _tracks[unit_offset]._data[i].getKey(0)
                    ){
                        ((Unit *)_tracks[unit_offset]._data[i].getPtr())->insertData(idx, data, 0, i);
                        
                        return;
                    }
                }
                ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, 0, _options.track_length);
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                for(int i = 0; i < _options.track_length; ++i){
                    for(int j = 0; j < _options.unit_size; ++j){
                        if(
                            _tracks[unit_offset]._data[i].getBitmap(j) &&
                            _tracks[unit_offset]._data[i].getPtr() != nullptr &&
                            idx < _tracks[unit_offset]._data[i].getKey(j)
                        ){
                            if(j == 1 && !((Unit *)_tracks[unit_offset]._data[i].getPtr())->isValid(1)){
                                continue;
                            }
                            ((Unit *)_tracks[unit_offset]._data[i].getPtr())->insertData(idx, data, j, i);
                            return;
                        }
                    }
                }
                ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, 0, _options.track_length);

                /*
                KeyPtrSet rightMostData = _tracks[unit_offset].getMaxData();
                if(rightMostData.getPtr() == getBackSideUnit(unit_offset)){
                    
                    if(((Unit *)_tracks[unit_offset]._sideBack)->_tracks[1].isValid()){
                        uint64_t next_unit_offset;
                        uint64_t maxIndexInternal = _tracks[unit_offset].getMaxIndex();
                        uint64_t maxIndexNext = getBackSideUnit(unit_offset)->_tracks[1].getMinIndex();
                        if(maxIndexInternal <= maxIndexNext){
                            ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, 1, _options.track_length);
                        }
                        else{
                            
                            if(getBackSideUnit(unit_offset)->isLeaf()){
                                getBackSideUnit(unit_offset)->getBackSideUnit(0)->insertData(idx, data, 0, _options.track_length);
                            }
                            else{
                                throw "test throw";
                            }
                            
                        }
                    }
                    else{
                        ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, 0, _options.track_length);
                    }
                }
                else{
                    ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, 0, _options.track_length);
                }*/

            }

            return;
        }
    }

    /**
     * * Insert data to current node
     * ! for leaf node
     * TODO evaluation
     * TODO check unit full, and then split to a new unit
     * TODO check one node of unit, and then split in unit
     * TODO in double tracks, probabaly balance operation
     * ? can merge with insertCurrentPoint()?
     */
    void insertCurrentData(uint64_t idx, uint64_t data, uint64_t unit_offset, uint64_t data_enter_offset){
        //
        //std::clog << "<log> <insertCurrentData()> data_enter_offset: " << data_enter_offset << std::endl;
        //
        
        Offset insertPosition = -1;

        if(!isLeaf())
            throw "This function is for Leaf node";

        bool insertSide = false;
        if(data_enter_offset == _options.track_length && !isRoot()){
            insertSide = true;
            data_enter_offset = getParentUnit()->_tracks[getParentOffset()].getRightMostOffset();
        }
        if(!isRoot() && getParentUnit()->getBackSideUnit() == this){
            insertSide = true;
        }
        
        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            if(isFull(unit_offset)){
                //* get the middle key and the pointer of new split unit
                KeyPtrSet promote = splitNode(idx, unit_offset);

                if(promote.getKey(0) == 0)
                    return;

                // need to grow this tree
                if(_isRoot){
                    setRoot(false);

                    // allocate a new root unit
                    Unit *newRoot = System::allocUnit(_options, false);
                    //* it must be internal unit
                    newRoot->setLeaf(false);

                    // new root unit insert current unit
                    newRoot->insertCurrentPointer(*promote.key, this, 0, 0);
                    this->connectParentUnit(newRoot, 0);

                    // new root unit insert new split unit
                    newRoot->connectBackSideUnit((Unit *)promote.ptr);
                    ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);

                    // side pointer connect to new split unit
                    if(promote.getPtr() != this){
                        connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                    }

                    // insert new index
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }

                // For leaf node, connect two side pointers
                if(hasBackSideUnit() && promote.getPtr() != getBackSideUnit()){
                    ((Unit *)promote.ptr)->connectBackSideUnit(getBackSideUnit());
                    getBackSideUnit()->connectFrontSideUnit(((Unit *)promote.ptr));
                }
                if(promote.getPtr() != this){
                    connectBackSideUnit((Unit *)promote.ptr);
                    ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                }

                //std::clog << "<log> this->_tracks[0]._parent: " << this->_tracks[0]._parent << std::endl;
                getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                // insert new index from root
                //? or from its parent
                getRoot()->insertData(idx, data, 0, -1);

                return;
            }
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            std::clog << "<log> <insertCurrentData()> idx: " << idx << std::endl;
            std::clog << "<log> <insertCurrentData()> _tracks[unit_offset]: " << _tracks[unit_offset] << std::endl;
            std::clog << "<log> <Unit::insertCurrentData()::UNIT> isFullUnit(): " << isFullUnit() << std::endl;
            std::clog << "<log> <insertCurrentData()> insertSide: " << insertSide << std::endl;

            if(isFull(unit_offset) && !isFullUnit()){
                if(isRoot()){
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    setRoot(false);

                    // allocate a new root unit
                    Unit *newRoot = System::allocUnit(_options, false);
                    //* it must be internal unit
                    newRoot->setLeaf(false);

                    // new root unit insert current unit
                    newRoot->insertCurrentPointer(*promote.key, this, 0, -1);
                    this->connectParentUnit(newRoot, 0);

                    // new root unit insert new split unit
                    newRoot->connectBackSideUnit((Unit *)promote.ptr);
                    ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);

                    // side pointer connect to new split unit
                    if(promote.getPtr() != this){
                        connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                    }

                    // insert new index
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
                else if(insertSide){
                    KeyPtrSet promote = splitNode(idx, unit_offset, insertSide);

                    if(hasBackSideUnit() && promote.getPtr() != getBackSideUnit()){
                        ((Unit *)promote.ptr)->connectBackSideUnit(getBackSideUnit());
                        getBackSideUnit()->connectFrontSideUnit(((Unit *)promote.ptr));
                    }
                    if(promote.getPtr() != this){
                        connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                    }

                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                    // insert new index from root
                    //? or from its parent
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
                else if(!isAllValid()){
                    //* get the middle key and the pointer of new split unit
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    if(promote.getKey(0) == 0)
                        return;

                    //std::clog << "<log> this->_tracks[0]._parent: " << this->_tracks[0]._parent << std::endl;
                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                    // insert new index from root
                    //? or from its parent
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
                else if(isAllValid() && !isFullUnit()){
                    std::clog << "<log> <Unit::insertCurrentData()::UNIT> isAllValid()" << std::endl;

                    Node *leftNode = nullptr, *rightNode = nullptr;
                    uint64_t mid = 0;

                    if(unit_offset < (unit_offset+1) % 2){
                        leftNode = &_tracks[unit_offset];
                        rightNode = &_tracks[(unit_offset+1) % 2];
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        balanceDataFromLeft(*leftNode, *rightNode);
                        
                        //std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[0]._data[enter_offset]: " << getParentUnit()->_tracks[0] << std::endl;
                        
                        mid = rightNode->getMinIndex();
                        if(mid < idx && rightNode->isFull()){
                            rightNode->_data[0].setKey(0, idx);
                            swap(mid, idx);
                        }
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }
                    else{
                        leftNode = &_tracks[(unit_offset+1) % 2];
                        rightNode = &_tracks[unit_offset];
                        balanceDataFromRight(*leftNode, *rightNode);
                        
                        mid = min(idx, rightNode->getMinIndex());
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }

                    
                    if(idx < mid){
                        leftNode->insertData(idx, new uint64_t(data), insertPosition, true);
                    }
                    else{
                        rightNode->insertData(idx, new uint64_t(data), insertPosition, true);
                    }

                    if(!isRoot()){
                        //std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[0]._data[enter_offset]: " << getParentUnit()->_tracks[0] << std::endl;
                        if(leftNode->_parentOffset != rightNode->_parentOffset){
                            uint64_t parentOffset = 1;
                            Unit *parent = getParentUnit(parentOffset);
                            while(!parent->isRoot() && mid < parent->_tracks[parentOffset]._data[0].getKey(0)){
                                parentOffset = parent->getParentOffset();
                                parent = parent->getParentUnit(parentOffset);
                            }

                            if(idx < mid){
                                for(int i = 0; i < _options.track_length; ++i){
                                    for(int j = 0; j < _options.unit_size; ++j){
                                        if(parent->_tracks[parentOffset]._data[i].getBitmap(j) && mid < parent->_tracks[parentOffset]._data[i].getKey(j)){
                                            parent->_tracks[parentOffset]._data[i].setKey(j, mid);
                                            std::clog << "<log> <insertCurrentData()> parent->_tracks[parentOffset]: " << parent->_tracks[parentOffset] << std::endl;
                                            return;
                                        }
                                    }
                                }
                            }
                            else{
                                for(int i = _options.track_length - 1; i >= 0; --i){
                                    for(int j = _options.unit_size - 1; j >= 0 ; --j){
                                        if(parent->_tracks[parentOffset]._data[i].getBitmap(j) && mid > parent->_tracks[parentOffset]._data[i].getKey(j)){
                                            parent->_tracks[parentOffset]._data[i].setKey(j, mid);
                                            std::clog << "<log> <insertCurrentData()> parent->_tracks[parentOffset]: " << parent->_tracks[parentOffset] << std::endl;
                                            return;
                                        }
                                    }
                                }
                            }

                            throw "balance error";
                        }
                        else{
                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].setKey(0, mid);
                        }
                        
                        //std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[getParentOffset()]: " << getParentUnit()->_tracks[getParentOffset()] << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }
                    
                    return;
                }
                
            }
            else if(isFullUnit()){
                std::clog << "<log> <Unit::insertCurrentData()::UNIT> isFullUnit()" << std::endl;

                /*if(hasBackSideUnit() && !getBackSideUnit()->isAllValid() && getParentUnit()->getBackSideUnit() != getBackSideUnit()){
                    //* get the middle key and the pointer of new split unit
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    if(promote.getKey(0) == 0)
                        return;

                    // For leaf node, connect two side pointers
                    if(hasBackSideUnit() && promote.getPtr() != getBackSideUnit()){
                        ((Unit *)promote.ptr)->connectBackSideUnit(getBackSideUnit());
                        getBackSideUnit()->connectFrontSideUnit(((Unit *)promote.ptr));
                    }
                    if(promote.getPtr() != this){
                        connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                    }

                    //std::clog << "<log> getParentUnit()->_tracks[getParentOffset()]: " << getParentUnit()->_tracks[getParentOffset()] << std::endl;
                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                    // insert new index from root
                    //? or from its parent
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
                else*/
                {
                    //* get the middle key and the pointer of new split unit
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    if(promote.getKey(0) == 0)
                        return;

                    // For leaf node, connect two side pointers
                    if(hasBackSideUnit() && promote.getPtr() != getBackSideUnit()){
                        ((Unit *)promote.ptr)->connectBackSideUnit(getBackSideUnit());
                        getBackSideUnit()->connectFrontSideUnit(((Unit *)promote.ptr));
                    }
                    if(promote.getPtr() != this){
                        connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                    }

                    //std::clog << "<log> getParentUnit()->_tracks[getParentOffset()]: " << getParentUnit()->_tracks[getParentOffset()] << std::endl;
                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                    // insert new index from root
                    //? or from its parent
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
                
            }
            /*
            //* if unit is not full, do balance
            if(isFull(unit_offset) && !isFullUnit()){
                //* if all valid, just need to balance
                if(isAllValid()){
                    Node *leftNode = nullptr, *rightNode = nullptr;
                    uint64_t mid = 0;

                    if(unit_offset < (unit_offset+1) % 2){
                        leftNode = &_tracks[unit_offset];
                        rightNode = &_tracks[(unit_offset+1) % 2];
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        balanceDataFromLeft(*leftNode, *rightNode);
                        
                        //std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[0]._data[enter_offset]: " << getParentUnit()->_tracks[0] << std::endl;
                        
                        mid = rightNode->getMinIndex();
                        if(mid < idx && rightNode->isFull()){
                            rightNode->_data[0].setKey(0, idx);
                            swap(mid, idx);
                        }
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }
                    else{
                        leftNode = &_tracks[(unit_offset+1) % 2];
                        rightNode = &_tracks[unit_offset];
                        balanceDataFromRight(*leftNode, *rightNode);
                        
                        mid = min(idx, rightNode->getMinIndex());
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }

                    
                    if(idx < mid){
                        leftNode->insertData(idx, new uint64_t(data), true);
                    }
                    else{
                        rightNode->insertData(idx, new uint64_t(data), true);
                    }

                    if(!isRoot()){
                        //std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[0]._data[enter_offset]: " << getParentUnit()->_tracks[0] << std::endl;
                        if(leftNode->_parentOffset != rightNode->_parentOffset){
                            uint64_t parentOffset = 1;
                            Unit *parent = getParentUnit(parentOffset);
                            while(!parent->isRoot() && mid < parent->_tracks[parentOffset]._data[0].getKey(0)){
                                parentOffset = parent->getParentOffset();
                                parent = parent->getParentUnit(parentOffset);
                            }

                            if(idx < mid){
                                for(int i = 0; i < _options.track_length; ++i){
                                    for(int j = 0; j < _options.unit_size; ++j){
                                        if(parent->_tracks[parentOffset]._data[i].getBitmap(j) && mid < parent->_tracks[parentOffset]._data[i].getKey(j)){
                                            parent->_tracks[parentOffset]._data[i].setKey(j, mid);
                                            std::clog << "<log> <insertCurrentData()> parent->_tracks[parentOffset]: " << parent->_tracks[parentOffset] << std::endl;
                                            return;
                                        }
                                    }
                                }
                            }
                            else{
                                for(int i = _options.track_length - 1; i >= 0; --i){
                                    for(int j = _options.unit_size - 1; j >= 0 ; --j){
                                        if(parent->_tracks[parentOffset]._data[i].getBitmap(j) && mid > parent->_tracks[parentOffset]._data[i].getKey(j)){
                                            parent->_tracks[parentOffset]._data[i].setKey(j, mid);
                                            std::clog << "<log> <insertCurrentData()> parent->_tracks[parentOffset]: " << parent->_tracks[parentOffset] << std::endl;
                                            return;
                                        }
                                    }
                                }
                            }

                            throw "balance error";
                        }
                        else{
                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].setKey(0, mid);
                        }
                        
                        //std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[getParentOffset()]: " << getParentUnit()->_tracks[getParentOffset()] << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }
                    
                    return;
                }
                //* else, need to promote a key
                else{
                    //* get the middle key and the pointer of new split unit
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    if(promote.getKey(0) == 0)
                        return;

                    // need to grow this tree
                    if(_isRoot){
                        setRoot(false);

                        // allocate a new root unit
                        Unit *newRoot = System::allocUnit(_options, false);
                        //* it must be internal unit
                        newRoot->setLeaf(false);

                        // new root unit insert current unit
                        newRoot->insertCurrentPointer(*promote.key, this, 0, -1);
                        this->connectParentUnit(newRoot, 0);

                        // new root unit insert new split unit
                        newRoot->connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);

                        // side pointer connect to new split unit
                        if(promote.getPtr() != this){
                            connectBackSideUnit((Unit *)promote.ptr);
                            ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                        }

                        // insert new index
                        getRoot()->insertData(idx, data, 0, -1);

                        return;
                    }

                    // For leaf node, connect two side pointers
                    if(hasBackSideUnit() && promote.getPtr() != getBackSideUnit()){
                        ((Unit *)promote.ptr)->connectBackSideUnit(getBackSideUnit());
                        getBackSideUnit()->connectFrontSideUnit(((Unit *)promote.ptr));
                    }
                    if(promote.getPtr() != this){
                        connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                    }

                    //std::clog << "<log> this->_tracks[0]._parent: " << this->_tracks[0]._parent << std::endl;
                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                    // insert new index from root
                    //? or from its parent
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
            }
            //* if unit full, need to do unit-unit split
            else if(isFullUnit()){
                //* get the middle key and the pointer of new split unit
                KeyPtrSet promote = splitNode(idx, unit_offset);

                if(promote.getKey(0) == 0)
                    return;

                // need to grow this tree
                if(_isRoot){
                    setRoot(false);

                    // allocate a new root unit
                    Unit *newRoot = System::allocUnit(_options, false);
                    //* it must be internal unit
                    newRoot->setLeaf(false);

                    // new root unit insert current unit
                    newRoot->insertCurrentPointer(*promote.key, this, 0, 0);
                    this->connectParentUnit(newRoot, 0);

                    // new root unit insert new split unit
                    newRoot->connectBackSideUnit((Unit *)promote.ptr);
                    ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);

                    // side pointer connect to new split unit
                    if(promote.getPtr() != this){
                        connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                    }

                    // insert new index
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }

                // For leaf node, connect two side pointers
                if(hasBackSideUnit() && promote.getPtr() != getBackSideUnit()){
                    ((Unit *)promote.ptr)->connectBackSideUnit(getBackSideUnit());
                    getBackSideUnit()->connectFrontSideUnit(((Unit *)promote.ptr));
                }
                if(promote.getPtr() != this){
                    connectBackSideUnit((Unit *)promote.ptr);
                    ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                }

                std::clog << "<log> getParentUnit()->_tracks[getParentOffset()]: " << getParentUnit()->_tracks[getParentOffset()] << std::endl;
                getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                // insert new index from root
                //? or from its parent
                getRoot()->insertData(idx, data, 0, -1);

                return;
            }
            */
        }

        //* direct insert index and data as data pointer to node
        _tracks[unit_offset].insertData(idx, new uint64_t(data), insertPosition);
        //std::clog << "<log> <insertCurrentData()> _tracks[unit_offset]: " << _tracks[unit_offset] << std::endl;
    }

    void insertCurrentPointer(uint64_t idx, Unit *unit, uint64_t unit_offset, uint64_t data_enter_offset){
        //
        //
        //std::clog << "<log> <insertCurrentPointer()> offset: " << unit_offset << std::endl;
        // 

        Offset insertPosition = -1;

        if(isLeaf())
            throw "This function is for Internal node";
        
        bool insertSide = false;
        if(data_enter_offset == _options.track_length && !isRoot()){
            insertSide = true;
            data_enter_offset = getParentUnit()->_tracks[getParentOffset()].getRightMostOffset();
        }
        if(!isRoot() && getParentUnit()->getBackSideUnit() == this){
            insertSide = true;
        }

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            if(isFull(unit_offset)){
                KeyPtrSet promote = splitNode(idx, unit_offset);

                if(_isRoot){
                    setRoot(false);

                    Unit *newRoot = System::allocUnit(_options, false);

                    newRoot->insertCurrentPointer(*promote.key, this, 0, 0);
                    this->connectParentUnit(newRoot, 0);

                    newRoot->connectBackSideUnit((Unit *)promote.ptr, 0);
                    ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);
                    
                    Unit *rightUnit = (Unit *)promote.ptr;
                    if(idx < promote.getKey(0)){
                        insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(this, 0);
                    }
                    else if(idx > promote.getKey(0)){
                        rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(rightUnit, 0);
                    }
                    else{ 
                        uint64_t leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                        rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                        unit->connectParentUnit(rightUnit, 0);
                    }

                    return;
                }

                getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0, 0);          

                Unit *rightUnit = (Unit *)promote.ptr;
                if(idx < promote.getKey(0)){
                    insertCurrentPointer(idx, unit, 0, 0);
                    unit->connectParentUnit(this, 0);
                }
                else if(idx > promote.getKey(0)){
                    rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                    unit->connectParentUnit(rightUnit, 0);
                }
                else{
                    uint64_t leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                    rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                    unit->connectParentUnit(rightUnit, 0);
                }

                return;
            }

            _tracks[0].insertData(idx, unit, insertPosition);
            unit->connectParentUnit(this, 0);
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            std::clog << "<log> <insertCurrentPointer()> idx: " << idx << std::endl;
            std::clog << "<log> <insertCurrentPointer()> unit: " << unit << std::endl;
            std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[unit_offset] << std::endl;
            std::clog << "<log> <insertCurrentPointer()> this: " << this << std::endl;
            std::clog << "<log> <insertCurrentPointer()> insertSide: " << insertSide << std::endl;
            // std::clog << "<log> <insertCurrentPointer()> isFullUnit(): " << isFullUnit() << std0::endl;
            // std::clog << "<log> <insertCurrentPointer()> isFull(unit_offset): " << isFull(unit_offset) << std::endl;
            // std::clog << "<log> <insertCurrentPointer()> isPossibleInsert(unit_offset, unit): " << isPossibleInsert(unit_offset, unit) << std::endl; 
            
            if(isFull(unit_offset) && !isPossibleInsert(unit_offset, unit)){
                if(isRoot()){
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    setRoot(false);

                    Unit *newRoot = System::allocUnit(_options, false);

                    newRoot->insertCurrentPointer(*promote.key, this, 0, 0);
                    this->connectParentUnit(newRoot, 0);

                    newRoot->connectBackSideUnit((Unit *)promote.ptr, 0);
                    ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);
                    
                    Unit *rightUnit = (Unit *)promote.ptr;
                    if(idx < promote.getKey(0)){
                        _tracks[0].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                        unit->connectParentUnit(this, 0);

                        // if(
                        //     insertPosition == 0 &&
                        //     _tracks[0]._data[insertPosition+1].getSize() == 2
                        // ){
                        //     Unit *left = (Unit *)_tracks[0]._data[0].getPtr();
                        //     Unit *right = (Unit *)_tracks[0]._data[1].getPtr();
                        //     Node temp = left->_tracks[0];
                        //     left->_tracks[0] = right->_tracks[0];
                        //     right->_tracks[0] = temp;
                        // }
                    }
                    else if(idx > promote.getKey(0)){
                        rightUnit->_tracks[0].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                        unit->connectParentUnit(rightUnit, 0);

                        // if(
                        //     insertPosition == 0 &&
                        //     rightUnit->_tracks[0]._data[insertPosition+1].getSize() == 2
                        // ){
                        //     Unit *left = (Unit *)rightUnit->_tracks[0]._data[0].getPtr();
                        //     Unit *right = (Unit *)rightUnit->_tracks[0]._data[1].getPtr();
                        //     Node temp = left->_tracks[0];
                        //     left->_tracks[0] = right->_tracks[0];
                        //     right->_tracks[0] = temp;
                        // }
                    }
                    else{ 
                        uint64_t leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                        rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                        unit->connectParentUnit(rightUnit, 0);

                        // throw "promote mid test 1";
                        if(getBackSideUnit()->getValidSize() == 2){
                            std::clog << "<log> <copyHalfNode()> unit: " << unit << std::endl;
                            std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[0]: " << unit->_tracks[0] << std::endl;
                            std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[1]: " << unit->_tracks[1] << std::endl;
                            throw "promote mid test 1";

                            getBackSideUnit()->migrateNode(getBackSideUnit()->_tracks[1], unit->_tracks[1], unit, 1);
                            getBackSideUnit()->_tracks[1].setValid(false);
                        } 
                    }


                    //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[unit_offset] << std::endl; 
                    return;
                }
                else if(insertSide){
                    KeyPtrSet promote = splitNode(idx, unit_offset, insertSide);

                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);          

                    Unit *rightUnit = (Unit *)promote.ptr;
                    if(idx < promote.getKey(0)){
                        _tracks[0].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                        unit->connectParentUnit(this, 0);

                        // if(
                        //     insertPosition == 0 &&
                        //     _tracks[0]._data[insertPosition+1].getSize() == 2
                        // ){
                        //     Unit *left = (Unit *)_tracks[0]._data[0].getPtr();
                        //     Unit *right = (Unit *)_tracks[0]._data[1].getPtr();
                        //     Node temp = left->_tracks[0];
                        //     left->_tracks[0] = right->_tracks[0];
                        //     right->_tracks[0] = temp;
                        // }
                    }
                    else if(idx > promote.getKey(0)){
                        rightUnit->_tracks[0].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                        unit->connectParentUnit(rightUnit, 0);

                        // if(
                        //     insertPosition == 0 &&
                        //     rightUnit->_tracks[0]._data[insertPosition+1].getSize() == 2
                        // ){
                        //     Unit *left = (Unit *)rightUnit->_tracks[0]._data[0].getPtr();
                        //     Unit *right = (Unit *)rightUnit->_tracks[0]._data[1].getPtr();
                        //     Node temp = left->_tracks[0];
                        //     left->_tracks[0] = right->_tracks[0];
                        //     right->_tracks[0] = temp;
                        // }
                    }
                    else{
                        uint64_t leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                        rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                        unit->connectParentUnit(rightUnit, 0);

                        if(getBackSideUnit()->getValidSize() == 2){
                            std::clog << "<log> <copyHalfNode()> unit: " << unit << std::endl;
                            std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[0]: " << unit->_tracks[0] << std::endl;
                            std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[1]: " << unit->_tracks[1] << std::endl;
                            throw "promote mid test 2";

                            getBackSideUnit()->migrateNode(getBackSideUnit()->_tracks[1], unit->_tracks[1], unit, 1);
                            getBackSideUnit()->_tracks[1].setValid(false);
                        }
                    }

                    return;
                }
                else if(!isAllValid()){
                    // throw "test";

                    KeyPtrSet promote = splitNode(idx, unit_offset, insertSide);

                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);          

                    Unit *rightUnit = (Unit *)promote.ptr;
                    if(this != rightUnit){
                        throw "error";
                    }

                    if(idx < promote.getKey(0)){
                        _tracks[0].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                        unit->connectParentUnit(this, 0);

                        // if(
                        //     insertPosition == 0 &&
                        //     _tracks[0]._data[insertPosition+1].getSize() == 2
                        // ){
                        //     Unit *left = (Unit *)_tracks[0]._data[0].getPtr();
                        //     Unit *right = (Unit *)_tracks[0]._data[1].getPtr();
                        //     Node temp = left->_tracks[0];
                        //     left->_tracks[0] = right->_tracks[0];
                        //     right->_tracks[0] = temp;
                        // }
                    }
                    else if(idx > promote.getKey(0)){
                        _tracks[1].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                        unit->connectParentUnit(this, 1);

                        // if(
                        //     insertPosition == 0 &&
                        //     _tracks[1]._data[insertPosition+1].getSize() == 2
                        // ){
                        //     Unit *left = (Unit *)_tracks[1]._data[0].getPtr();
                        //     Unit *right = (Unit *)_tracks[1]._data[1].getPtr();
                        //     Node temp = left->_tracks[0];
                        //     left->_tracks[0] = right->_tracks[0];
                        //     right->_tracks[0] = temp;
                        // }
                    }
                    else{
                        uint64_t leftMostOffset = _tracks[1].getLeftMostOffset();
                        _tracks[1]._data[leftMostOffset].setPtr(unit);
                        unit->connectParentUnit(this, 1);

                        if(getBackSideUnit()->getValidSize() == 2){
                            std::clog << "<log> <copyHalfNode()> unit: " << unit << std::endl;
                            std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[0]: " << unit->_tracks[0] << std::endl;
                            std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[1]: " << unit->_tracks[1] << std::endl;
                            throw "promote mid test 3";

                            getBackSideUnit()->migrateNode(getBackSideUnit()->_tracks[1], unit->_tracks[1], unit, 1);
                            getBackSideUnit()->_tracks[1].setValid(false);
                        }
                    }

                    std::clog << "<log> <insertCurrentPointer()> _tracks[1]: " << _tracks[1] << std::endl;
                    
                    return;
                }
                else{
                    // throw "test";

                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    std::clog << "<log> <insertCurrentPointer()> _tracks[offset] 4: " << _tracks[unit_offset] << std::endl;

                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);          

                    Unit *rightUnit = (Unit *)promote.ptr;
                    if(unit_offset == 0){
                        if(idx < promote.getKey(0)){
                            _tracks[0].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                            unit->connectParentUnit(this, 0);

                            // if(
                            //     insertPosition == 0 &&
                            //     _tracks[unit_offset]._data[insertPosition+1].getSize() == 2
                            // ){
                            //     Unit *left = (Unit *)_tracks[unit_offset]._data[0].getPtr();
                            //     Unit *right = (Unit *)_tracks[unit_offset]._data[1].getPtr();
                            //     Node temp = left->_tracks[0];
                            //     left->_tracks[0] = right->_tracks[0];
                            //     right->_tracks[0] = temp;
                            // }
                        }
                        else if(idx > promote.getKey(0)){
                            _tracks[1].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                            unit->connectParentUnit(this, 1);

                            // if(
                            //     insertPosition == 0 &&
                            //     rightUnit->_tracks[0]._data[insertPosition+1].getSize() == 2
                            // ){
                            //     Unit *left = (Unit *)rightUnit->_tracks[0]._data[0].getPtr();
                            //     Unit *right = (Unit *)rightUnit->_tracks[0]._data[1].getPtr();
                            //     Node temp = left->_tracks[0];
                            //     left->_tracks[0] = right->_tracks[0];
                            //     right->_tracks[0] = temp;
                            // }
                        }
                        else{
                            uint64_t leftMostOffset = _tracks[1].getLeftMostOffset();
                            _tracks[1]._data[leftMostOffset].setPtr(unit);
                            unit->connectParentUnit(this, 1);

                            if(getBackSideUnit()->getValidSize() == 2){
                                std::clog << "<log> <copyHalfNode()> unit: " << unit << std::endl;
                                std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[0]: " << unit->_tracks[0] << std::endl;
                                std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[1]: " << unit->_tracks[1] << std::endl;
                                throw "promote mid test 4";

                                getBackSideUnit()->migrateNode(getBackSideUnit()->_tracks[1], unit->_tracks[1], unit, 1);
                                getBackSideUnit()->_tracks[1].setValid(false);
                            }
                        }
                    }
                    else if(unit_offset == 1){
                        if(idx < promote.getKey(0)){
                            _tracks[unit_offset].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                            unit->connectParentUnit(this, unit_offset);

                            // if(
                            //     insertPosition == 0 &&
                            //     _tracks[unit_offset]._data[insertPosition+1].getSize() == 2
                            // ){
                            //     Unit *left = (Unit *)_tracks[unit_offset]._data[0].getPtr();
                            //     Unit *right = (Unit *)_tracks[unit_offset]._data[1].getPtr();
                            //     Node temp = left->_tracks[0];
                            //     left->_tracks[0] = right->_tracks[0];
                            //     right->_tracks[0] = temp;
                            // }
                        }
                        else if(idx > promote.getKey(0)){
                            rightUnit->_tracks[0].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMaxIndex());
                            unit->connectParentUnit(rightUnit, 0);

                            // if(
                            //     insertPosition == 0 &&
                            //     rightUnit->_tracks[0]._data[insertPosition+1].getSize() == 2
                            // ){
                            //     Unit *left = (Unit *)rightUnit->_tracks[0]._data[0].getPtr();
                            //     Unit *right = (Unit *)rightUnit->_tracks[0]._data[1].getPtr();
                            //     Node temp = left->_tracks[0];
                            //     left->_tracks[0] = right->_tracks[0];
                            //     right->_tracks[0] = temp;
                            // }
                        }
                        else{
                            uint64_t leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                            rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                            unit->connectParentUnit(rightUnit, 0);

                            if(getBackSideUnit()->getValidSize() == 2){
                                std::clog << "<log> <copyHalfNode()> unit: " << unit << std::endl;
                                std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[0]: " << unit->_tracks[0] << std::endl;
                                std::clog << "<log> <copyHalfNode()> ((Unit *)promote.getPtr())->_tracks[1]: " << unit->_tracks[1] << std::endl;
                                throw "promote mid test 4";

                                getBackSideUnit()->migrateNode(getBackSideUnit()->_tracks[1], unit->_tracks[1], unit, 1);
                                getBackSideUnit()->_tracks[1].setValid(false);
                            }
                        }
                    }
                    
                    
                    return;
                }
            }
            /*
            if(isFull(unit_offset) && !isPossibleInsert(unit_offset, unit)){
                {
                    KeyPtrSet promote = splitNode(idx, unit_offset);
                    //uint64_t wait_insert_index = _tracks[0].getMaxIndex();
                    //KeyPtrSet promote = splitNode(wait_insert_index, unit_offset);

                    if(_isRoot){
                        setRoot(false);

                        Unit *newRoot = System::allocUnit(_options, false);

                        newRoot->insertCurrentPointer(*promote.key, this, 0, 0);
                        this->connectParentUnit(newRoot, 0);

                        newRoot->connectBackSideUnit((Unit *)promote.ptr, 0);
                        ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);
                        
                        Unit *rightUnit = (Unit *)promote.ptr;

                        if(idx < promote.getKey(0)){
                            _tracks[0].insertData(idx, unit, true);
                            unit->connectParentUnit(this, 0);
                        }
                        else if(idx > promote.getKey(0)){
                            _tracks[1].insertData(idx, unit, true);
                            unit->connectParentUnit(this, 1);
                        }
                        else{ 
                            uint64_t leftMostOffset = _tracks[1].getLeftMostOffset();
                            _tracks[1]._data[leftMostOffset].setPtr(unit);
                            unit->connectParentUnit(this, 1);
                        }

                        //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[unit_offset] << std::endl; 
                        return;
                    }

                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                    Unit *rightUnit = (Unit *)promote.ptr;
                    if(this == rightUnit){
                        //std::clog << "<log> <insertCurrentPointer()> _tracks[0]: " << _tracks[0] << std::endl; 
                        //std::clog << "<log> <insertCurrentPointer()> _tracks[1]: " << _tracks[1] << std::endl; 
                        if(idx < promote.getKey(0)){
                            _tracks[0].insertData(idx, unit, true);
                            unit->connectParentUnit(this, 0);
                        }
                        else if(idx > promote.getKey(0)){
                            _tracks[1].insertData(idx, unit, true);
                            unit->connectParentUnit(this, 1);
                        }
                        else{ 
                            uint64_t leftMostOffset = _tracks[1].getLeftMostOffset();
                            _tracks[1]._data[leftMostOffset].setPtr(unit);
                            unit->connectParentUnit(this, 1);
                        }
                    }
                    else{
                        std::clog << "<log> <insertCurrentPointer()> _tracks[1]: " << _tracks[1] << std::endl; 
                        std::clog << "<log> <insertCurrentPointer()> rightUnit->_tracks[0]: " << rightUnit->_tracks[0] << std::endl;
                        std::clog << "<log> <insertCurrentPointer()> idx: " << idx << std::endl; 
                        std::clog << "<log> <insertCurrentPointer()> promote.getKey(0): " << promote.getKey(0) << std::endl;  
                        if(unit_offset == 0){
                            if(idx < promote.getKey(0)){
                                _tracks[0].insertData(idx, unit, true);
                                unit->connectParentUnit(this, 0);
                            }
                            else if(idx > promote.getKey(0)){
                                _tracks[1].insertData(idx, unit, true);
                                unit->connectParentUnit(this, 1);
                            }
                            else{ 
                                uint64_t leftMostOffset = _tracks[1].getLeftMostOffset();
                                _tracks[1]._data[leftMostOffset].setPtr(unit);
                                unit->connectParentUnit(this, 1);
                            }
                        }
                        else if(unit_offset == 1){
                            if(idx < promote.getKey(0)){
                            _tracks[1].insertData(idx, unit, true);
                            unit->connectParentUnit(this, 1);
                            }
                            else if(idx > promote.getKey(0)){
                                rightUnit->_tracks[0].insertData(idx, unit, true);
                                unit->connectParentUnit(this, 0);
                            }
                            else{ 
                                uint64_t leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                                rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                                unit->connectParentUnit(this, 0);
                            }
                        }
                        
                    }

                    return;
                }
                
            }
            */

            /*
            //* if unit is not full, do balance
            if(isFull(unit_offset) && !isFullUnit() && !isPossibleInsert(unit_offset, unit)){
                if(isAllValid()){
                    Node *leftNode = nullptr, *rightNode = nullptr;

                    if(unit_offset < (unit_offset+1) % 2){
                        leftNode = &_tracks[unit_offset];
                        rightNode = &_tracks[(unit_offset+1) % 2];
                        balanceDataFromLeft(*leftNode, *rightNode);
                        if(!isRoot()){
                            uint64_t temp = getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].getKey(0);
                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].setKey(0, rightNode->getMinIndex());
                            rightNode->_data[0].setKey(0, temp);
                            //std::clog << "<log> <insertCurrentPointer()> rightNode: " << *rightNode << std::endl;
                            //std::clog << "<log> <insertCurrentPointer()> getParentUnit()->_tracks[getParentOffset()]: " << getParentUnit()->_tracks[getParentOffset()] << std::endl;
                        }
                        leftNode->insertData(idx, unit, true);
                        unit->connectParentUnit(this, 0);
                        std::clog << "<log> <insertCurrentPointer()> leftNode: " << *leftNode << std::endl;
                    }
                    else{
                        leftNode = &_tracks[(unit_offset+1) % 2];
                        rightNode = &_tracks[unit_offset];
                        balanceDataFromRight(*leftNode, *rightNode);

                        if(!isRoot()){
                            uint64_t temp = getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].getKey(0);
                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].setKey(0, rightNode->getMinIndex());
                            rightNode->_data[0].setKey(0, temp);
                        }
                        rightNode->insertData(idx, unit, true);
                        unit->connectParentUnit(this, 1);
                    }
                    
                    
                    return;
                }
                else{
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    if(_isRoot){
                        setRoot(false);

                        Unit *newRoot = System::allocUnit(_options, false);

                        newRoot->insertCurrentPointer(*promote.key, this, 0, 0);
                        this->connectParentUnit(newRoot, 0);

                        newRoot->connectBackSideUnit((Unit *)promote.ptr, 0);
                        ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);
                        
                        Unit *rightUnit = (Unit *)promote.ptr;

                        if(idx < promote.getKey(0)){
                            _tracks[0].insertData(idx, unit, true);
                            unit->connectParentUnit(this, 0);
                        }
                        else if(idx > promote.getKey(0)){
                            _tracks[1].insertData(idx, unit, true);
                            unit->connectParentUnit(this, 1);
                        }
                        else{ 
                            uint64_t leftMostOffset = _tracks[1].getLeftMostOffset();
                            _tracks[1]._data[leftMostOffset].setPtr(unit);
                            unit->connectParentUnit(this, 1);
                        }

                        //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[unit_offset] << std::endl; 
                        return;
                    }

                    if(idx < promote.getKey(0)){
                        _tracks[0].insertData(idx, unit, true);
                        unit->connectParentUnit(this, 0);
                    }
                    else if(idx > promote.getKey(0)){
                        _tracks[1].insertData(idx, unit, true);
                        unit->connectParentUnit(this, 1);
                    }
                    else{ 
                        uint64_t leftMostOffset = _tracks[1].getLeftMostOffset();
                        _tracks[1]._data[leftMostOffset].setPtr(unit);
                        unit->connectParentUnit(this, 1);
                    }

                    return;
                }
            }
            else if(isFullUnit() && !isPossibleInsert(unit_offset, unit)){
                std::clog << "<log> <insertCurrentPointer()> unit-unit split " << std::endl; 
                

                uint64_t wait_insert_index = _tracks[0].getMaxIndex();
                KeyPtrSet promote = splitNode(wait_insert_index, unit_offset);

                std::clog << "<log> <insertCurrentPointer()> promote: " << promote << std::endl;

                if(promote.getKey(0) == 0)
                    return;

                if(_isRoot){
                    setRoot(false);

                    Unit *newRoot = System::allocUnit(_options, false);

                    newRoot->insertCurrentPointer(*promote.key, this, 0, 0);
                    this->connectParentUnit(newRoot, 0);

                    newRoot->connectBackSideUnit((Unit *)promote.ptr, 0);
                    ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);
                    
                    Unit *rightUnit = (Unit *)promote.ptr;

                    if(idx < promote.getKey(0)){
                        insertCurrentPointer(idx, unit, unit_offset, 0);
                        unit->connectParentUnit(this, 1);
                    }
                    else if(idx > promote.getKey(0)){
                        rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(rightUnit, 0);
                    }
                    else{ 
                        uint64_t leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                        rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                        unit->connectParentUnit(rightUnit, 0);
                    }
                    return;
                }

                getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                Unit *rightUnit = (Unit *)promote.ptr;
                if(idx < promote.getKey(0)){
                    insertCurrentPointer(idx, unit, 1, 0);
                    unit->connectParentUnit(this, 1);
                }
                else if(idx > promote.getKey(0)){
                    rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                    unit->connectParentUnit(rightUnit, 0);
                }
                else{
                    uint64_t leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                    rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                    //((Unit *)rightUnit->_tracks[0]._data[0].getPtr())->connectParentUnit(rightUnit);
                    unit->connectParentUnit(rightUnit, 0);
                    //std::clog << "<log> rightUnit->_tracks[0]._data[0].getPtr(): " << rightUnit->_tracks[0]._data[0].getPtr() << std::endl;
                    //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
                }

                return;
            }
            */
            if(hasBackSideUnit()){
                //std::clog << "<log> <insertCurrentPointer()> idx: " << idx << std::endl;
                //std::clog << "<log> <insertCurrentPointer()> getBackSideUnit()->_tracks[0].getMaxIndex(): " << getBackSideUnit()->_tracks[0].getMaxIndex()<< std::endl;
                _tracks[unit_offset].insertData(idx, unit, insertPosition, idx > getBackSideUnit()->_tracks[0].getMinIndex());
            }
            else{
                _tracks[unit_offset].insertData(idx, unit, insertPosition);
            }
            unit->connectParentUnit(this, unit_offset);
        }
        
        //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[offset] << std::endl; 
        
        
        
        //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[unit_offset] << std::endl; 
    }

    void connectBackSideUnit(Unit *unit, uint64_t unit_offset = 0){
        _tracks[unit_offset].connectBackSideUnit(unit);
    }

    void connectFrontSideUnit(Unit *unit, uint64_t unit_offset = 0){
        _tracks[unit_offset].connectFrontSideUnit(unit);
    }

    bool hasBackSideUnit(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._sideBackBitmap;
    }

    bool hasFrontSideUnit(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._sideFrontBitmap;
    }

    Unit *getBackSideUnit(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._sideBack;
    }

    Unit *getFrontSideUnit(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._sideFront;
    }

    /**
     * * DONE
    */
    void connectParentUnit(Unit *unit, uint64_t parent_unit_offset = 0, uint64_t unit_offset = -1){
        if(unit_offset == -1){
            for(int i = 0; i < _options.unit_size; ++i){
                _tracks[i].connectParentNode(unit, parent_unit_offset);
            }
        }
        else{
            _tracks[unit_offset].connectParentNode(unit, parent_unit_offset);
        }
        
    }

    Unit *getParentUnit(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._parent;
    }

    uint64_t getParentOffset(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._parentOffset;
    }

    //? no use
    /*
    Unit *getParentRightUnit(uint64_t idx) const{
        for(int i = 0; i < _options.track_length-1; ++i){
            if(getParentUnit()->_tracks[0]._bitmap[i+1] && idx < getParentUnit()->_tracks[0]._data[i+1].getKey(0)){
                return (Unit *)_tracks[0]._parent->_tracks[0]._data[i+1].getPtr();
            }
        }
        return _tracks[0]._parent->getSideUnit();
    }*/
    
    /**
     * * Split half number of indices to a new unit
     * * In double tracks, in-unit split is balance operation
     * ? rename to splitUnit()
     * TODO UNIT: in-unit split
     * TODO UNIT: unit-to-unit split
     * TODO TRAD: in double tracks, new node may be allocated to the same unit
     * ! in UNIT method, unit_offset should be the right most (for easy implement)
     */
    KeyPtrSet splitNode(uint64_t wait_insert_idx, uint64_t unit_offset, bool insertSide = false){
        KeyPtrSet promote(2);

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            // split operation will generate same level unit/node
            Unit *newUnit = System::allocUnit(_options, isLeaf());

            // it is never root
            newUnit->setRoot(false);

            // find the middle index
            uint64_t promoteKey = System::getMid(_tracks[0]._data, _options.track_length, wait_insert_idx);
            
            // combine as a key-point set
            promote.setPtr(newUnit);
            promote.addKey(promoteKey);
            
            // copying data
            copyHalfNode(_tracks[0], newUnit->_tracks[0], promote, wait_insert_idx);
            //std::clog << "<log> _tracks[unit_offset]: " << _tracks[unit_offset] << std::endl;
            //std::clog << "<log> newUnit->_tracks[0]: " << newUnit->_tracks[0] << std::endl;

            // sure to have correct parent pointers
            if(!isLeaf()){
                getBackSideUnit()->connectParentUnit(this);
                newUnit->getBackSideUnit()->connectParentUnit(newUnit);
            }
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            
            if(isRoot() || insertSide){
                // split operation will generate same level unit/node
                Unit *newUnit = System::allocUnit(_options, isLeaf());

                // find the middle index
                uint64_t promoteKey = System::getMid(_tracks[0]._data, _options.track_length, wait_insert_idx);

                // combine as a key-point set
                promote.setPtr(newUnit);
                promote.addKey(promoteKey);

                // it is never root
                newUnit->setRoot(false);

                // split operation will generate same level unit/node
                if(!isLeaf()) newUnit->setLeaf(false);

                //* copying data
                copyHalfNode(_tracks[0], newUnit->_tracks[0], promote, wait_insert_idx);
                newUnit->_tracks[0].setValid(true);

                //* sure to have correct parent pointers
                if(!isLeaf()){
                    getBackSideUnit()->connectParentUnit(this, 0);
                    newUnit->getBackSideUnit()->connectParentUnit(newUnit, 0);
                }
            }
            //* In-Unit Split
            else if(!isAllValid()){
                // find the middle index
                uint64_t promoteKey = System::getMid(_tracks[0]._data, _options.track_length, wait_insert_idx);

                // combine as a key-point set
                /*
                if(!isLeaf() && !isPossibleInsert(unit_offset, this)){
                    promote.setPtr(getBackSideUnit());
                }
                else{
                    promote.setPtr(this);
                }
                */
                promote.setPtr(this);
                promote.addKey(promoteKey);

                copyHalfNode(_tracks[0], _tracks[1], promote, wait_insert_idx);
                _tracks[1].setValid(true);                
            }
            //* Unit-Unit Split
            else{
                if(isLeaf()){
                    //std::clog << "<log> getParentUnit()->getValidSize(): " << getParentUnit()->getValidSize() << std::endl;
                    /*if(!isRoot() && hasBackSideUnit() && !getBackSideUnit()->isAllValid() && getParentUnit()->getBackSideUnit() != getBackSideUnit()){
                        // throw "test";
                        getBackSideUnit()->migrateNode(getBackSideUnit()->_tracks[0], getBackSideUnit()->_tracks[1]);

                        uint64_t promoteKey = System::getMid(_tracks[1]._data, _options.track_length, wait_insert_idx);
                    
                        promote.setPtr(getBackSideUnit());
                        promote.addKey(promoteKey);

                        // copying data
                        copyHalfNode(_tracks[1], getBackSideUnit()->_tracks[0], promote, wait_insert_idx);
                    }
                    else*/
                    {
                        // split operation will generate same level unit/node
                        Unit *newUnit = System::allocUnit(_options, isLeaf());

                        // find the middle index
                        uint64_t promoteKey = System::getMid(_tracks[1]._data, _options.track_length, wait_insert_idx);

                        // combine as a key-point set
                        promote.setPtr(newUnit);
                        promote.addKey(promoteKey);

                        // it is never root
                        newUnit->setRoot(false);

                        // split operation will generate same level unit/node
                        if(!isLeaf()) newUnit->setLeaf(false);

                        //* copying data
                        copyHalfNode(_tracks[1], newUnit->_tracks[0], promote, wait_insert_idx);
                        newUnit->_tracks[0].setValid(true);
                        //std::clog << "<log> newUnit->_tracks[0]: " << newUnit->_tracks[0] << std::endl;
                    }
                }
                else{
                    // throw "Internal split developing";
                    std::clog << "<log> unit_offset: " << unit_offset << std::endl;
                    if(unit_offset == 0){
                        {
                            // split operation will generate same level unit/node
                            Unit *newUnit = System::allocUnit(_options, isLeaf());

                            // find the middle index
                            uint64_t promoteKey = System::getMid(_tracks[0]._data, _options.track_length, wait_insert_idx);

                            // combine as a key-point set
                            promote.setPtr(newUnit);
                            promote.addKey(promoteKey);

                            // it is never root
                            newUnit->setRoot(false);

                            // split operation will generate same level unit/node
                            if(!isLeaf()) newUnit->setLeaf(false);

                            //* copying data
                            migrateNode(_tracks[1], newUnit->_tracks[0], newUnit, 0);
                            newUnit->_tracks[0].setValid(true);
                            promote.setPtr(this);
                            copyHalfNode(_tracks[0], _tracks[1], promote, wait_insert_idx);
                            promote.setPtr(newUnit);

                            //* sure to have correct parent pointers
                            if(!isLeaf()){
                                getBackSideUnit(0)->connectParentUnit(this, 0);
                                newUnit->getBackSideUnit()->connectParentUnit(newUnit);
                            }
                            // throw "test 0";
                        }
                    }
                    else if(unit_offset == 1){
                        {
                            // split operation will generate same level unit/node
                            Unit *newUnit = System::allocUnit(_options, isLeaf());

                            // find the middle index
                            uint64_t promoteKey = System::getMid(_tracks[1]._data, _options.track_length, wait_insert_idx);

                            // combine as a key-point set
                            promote.setPtr(newUnit);
                            promote.addKey(promoteKey);

                            // it is never root
                            newUnit->setRoot(false);

                            // split operation will generate same level unit/node
                            if(!isLeaf()) newUnit->setLeaf(false);

                            //* copying data
                            copyHalfNode(_tracks[1], newUnit->_tracks[0], promote, wait_insert_idx);
                            newUnit->_tracks[0].setValid(true);
                            //std::clog << "<log> newUnit->_tracks[0]: " << newUnit->_tracks[0] << std::endl;

                            //* sure to have correct parent pointers
                            if(!isLeaf()){
                                getBackSideUnit(1)->connectParentUnit(this, 1);
                                newUnit->getBackSideUnit()->connectParentUnit(newUnit);
                            }
                        }
                    }
                }

            }
        }
        else{
            throw "undefined split operation";
        }

        return promote;
    }

    /**
     * * Copy half data from source to destination
     * TODO evaluation
     */
    void copyHalfNode(Node &source, Node &destination, KeyPtrSet &promote, uint64_t wait_insert_idx){
        //std::clog << "<log> <copyHalfNode()>" << std::endl;
        if(isLeaf()){
            switch (_options.read_mode){
            case Options::read_function::SEQUENTIAL:
                source._shiftCounter.count(2 * _options.word_length * (_options.track_length / 2));
                source._shiftCounter.count(2 * _options.word_length * (_options.track_length / 2));
                source._readCounter.count(_options.word_length * (_options.track_length / 2));
                break;
            case Options::read_function::RANGE_READ:
                source._shiftCounter.count(2 * _options.word_length);
                source._readCounter.count(_options.word_length * (_options.track_length / 2));
                break;
            default:
                throw "undefined read operation";
                break;
            }

            if(_options.node_ordering == Options::ordering::SORTED){
                if(wait_insert_idx < promote.getKey(0)){
                    for(int i = _options.track_length / 2 - 1, j = 0; i < _options.track_length; ++i, ++j){
                        destination._data[j] = source._data[i];
                        source._data[i].delAll();

                        switch (_options.update_mode){
                            case Options::update_function::OVERWRITE:
                                destination._shiftCounter.count(2 * _options.word_length);
                                destination._removeCounter.count(2 * _options.word_length);
                                destination._insertCounter.count(Evaluation::countSkyrmion(destination._data[j].getKey(0)));
                                destination._insertCounter.count(Evaluation::countSkyrmion((uint64_t)destination._data[j].getPtr()));
                                destination._shiftCounter.count(2 * _options.word_length);
                                break;
                            case Options::update_function::PERMUTATION_WRITE:
                                //TODO evaluation 
                                break;
                            case Options::update_function::PERMUTE_WORD_COUNTER:
                                //TODO evaluation 
                                break;
                            case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                                //TODO evaluation 
                                break;
                            default:
                                throw "undefined update operation";
                                break;
                        }
                    }
                }
                else{
                    for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                        destination._data[j] = source._data[i];
                        source._data[i].delAll();

                        switch (_options.update_mode){
                            case Options::update_function::OVERWRITE:
                                destination._shiftCounter.count(2 * _options.word_length);
                                destination._removeCounter.count(2 * _options.word_length);
                                destination._insertCounter.count(Evaluation::countSkyrmion(destination._data[j].getKey(0)));
                                destination._insertCounter.count(Evaluation::countSkyrmion((uint64_t)destination._data[j].getPtr()));
                                destination._shiftCounter.count(2 * _options.word_length);
                                break;
                            case Options::update_function::PERMUTATION_WRITE:
                                //TODO evaluation 
                                break;
                            case Options::update_function::PERMUTE_WORD_COUNTER:
                                //TODO evaluation 
                                break;
                            case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                                //TODO evaluation 
                                break;
                            default:
                                throw "undefined update operation";
                                break;
                        }
                    }
                }
            }
            else if(_options.node_ordering == Options::ordering::UNSORTED){
                throw "unsorted split developing";
            }

            destination.setValid(true);

            std::clog << "<log> <copyHalfNode()> Leaf source: " << source << std::endl;
            std::clog << "<log> <copyHalfNode()> Leaf destination: " << destination << std::endl;
        }
        else{
            switch (_options.read_mode){
            case Options::read_function::SEQUENTIAL:
                source._shiftCounter.count(2 * _options.word_length * (_options.track_length / 2));
                source._shiftCounter.count(2 * _options.word_length * (_options.track_length / 2));
                source._readCounter.count(_options.word_length * (_options.track_length / 2));
                break;
            case Options::read_function::RANGE_READ:
                source._shiftCounter.count(2 * _options.word_length);
                source._readCounter.count(_options.word_length * (_options.track_length / 2));
                break;
            default:
                throw "undefined read operation";
                break;
            }

            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                uint64_t mid = _options.track_length / 2;
                if(wait_insert_idx < promote.getKey(0)){
                    for(int i = _options.track_length / 2 - 1, j = 0; i < _options.track_length; ++i, ++j){
                        destination._data[j] = source._data[i];
                        source._data[i].delAll();

                        switch (_options.update_mode){
                            case Options::update_function::OVERWRITE:
                                destination._shiftCounter.count(2 * _options.word_length);
                                destination._removeCounter.count(2 * _options.word_length);
                                destination._insertCounter.count(Evaluation::countSkyrmion(destination._data[j].getKey(0)));
                                destination._insertCounter.count(Evaluation::countSkyrmion((uint64_t)destination._data[j].getPtr()));
                                destination._shiftCounter.count(2 * _options.word_length);
                                break;
                            case Options::update_function::PERMUTATION_WRITE:
                                //TODO evaluation 
                                break;
                            case Options::update_function::PERMUTE_WORD_COUNTER:
                                //TODO evaluation 
                                break;
                            case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                                //TODO evaluation 
                                break;
                            default:
                                throw "undefined update operation";
                                break;
                        }

                        ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr());
                    }
                }
                else{
                    for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                        destination._data[j] = source._data[i];
                        source._data[i].delAll();

                        switch (_options.update_mode){
                            case Options::update_function::OVERWRITE:
                                destination._shiftCounter.count(2 * _options.word_length);
                                destination._removeCounter.count(2 * _options.word_length);
                                destination._insertCounter.count(Evaluation::countSkyrmion(destination._data[j].getKey(0)));
                                destination._insertCounter.count(Evaluation::countSkyrmion((uint64_t)destination._data[j].getPtr()));
                                destination._shiftCounter.count(2 * _options.word_length);
                                break;
                            case Options::update_function::PERMUTATION_WRITE:
                                //TODO evaluation 
                                break;
                            case Options::update_function::PERMUTE_WORD_COUNTER:
                                //TODO evaluation 
                                break;
                            case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                                //TODO evaluation 
                                break;
                            default:
                                throw "undefined update operation";
                                break;
                        }

                        ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr());
                    }
                }
                destination.connectBackSideUnit(source._sideBack);

                bool promoteMid = true;
                for(int i = 0; i < _options.track_length; ++i){
                    if(source._data[i].getBitmap(0) && source._data[i].getKey(0) == promote.getKey(0)){
                        source.deleteMark(i);
                        promoteMid = false;
                        source.connectBackSideUnit((Unit *)source._data[i].getPtr());
                        //std::clog << "<log> <copyHalfNode()> source" << std::endl;
                    }
                }
                for(int i = 0; i < _options.track_length; ++i){
                    if(destination._data[i].getBitmap(0) && destination._data[i].getKey(0) == promote.getKey(0)){
                        destination.deleteMark(i);
                        promoteMid = false;
                        source.connectBackSideUnit((Unit *)destination._data[0].getPtr());
                        //std::clog << "<log> <copyHalfNode()> destination" << std::endl;
                    }                 
                }

                if(promoteMid){
                    source.connectBackSideUnit((Unit *)destination._data[0].getPtr());
                    //std::clog << "<log> <copyHalfNode()> promoteMid" << std::endl;
                    //std::clog << "<log> <copyHalfNode()> destination._data[0].getPtr(): " << destination._data[0].getPtr() << std::endl;
                }
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                std::clog << "<log> <copyHalfNode()> Internal source begin: " << source << std::endl;
                std::clog << "<log> <copyHalfNode()> Internal destination begin: " << destination << std::endl;

                bool inUnit = isFull(0) && !isFull(1) && !isRoot() && getParentUnit()->getBackSideUnit() != this;

                uint64_t mid = _options.track_length / 2;
                if(wait_insert_idx < promote.getKey(0)){
                    for(int i = _options.track_length / 2 - 1, j = 0; i < _options.track_length; ++i, ++j){
                        destination._data[j] = source._data[i];
                        source._data[i].delAll();

                        if(inUnit){
                            ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr(), 1);
                        }
                        else{
                            ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr(), 0);
                        }
                    }
                }
                else{
                    for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                        destination._data[j] = source._data[i];
                        source._data[i].delAll();

                        if(inUnit){
                            ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr(), 1);
                        }
                        else{
                            ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr(), 0);
                        }
                    }
                }
                destination.connectBackSideUnit(source._sideBack);
                if(inUnit){
                    destination._sideBack->connectParentUnit((Unit *)promote.getPtr(), 1);
                }
                else{
                    destination._sideBack->connectParentUnit((Unit *)promote.getPtr(), 0);
                }
                

                bool promoteMid = true;
                /*
                for(int i = 0; i < _options.track_length; ++i){
                    for(int j = 0; j < _options.unit_size; ++j){
                        if(source._data[i].getBitmap(j) && source._data[i].getKey(j) == promote.getKey(0)){
                            source.deleteData(promote.getKey(0));
                            promoteMid = false;
                            source.connectBackSideUnit((Unit *)source._data[source.getRightMostOffset()].getPtr());
                        }
                    }
                }
                for(int i = 0; i < _options.track_length; ++i){
                    for(int j = 0; j < _options.unit_size; ++j){
                        if(destination._data[i].getBitmap(j) && destination._data[i].getKey(j) == promote.getKey(0)){
                            destination.deleteData(promote.getKey(0));
                            promoteMid = false;
                            //source.connectBackSideUnit((Unit *)destination._data[0].getPtr());
                            source.connectBackSideUnit((Unit *)source._data[source.getRightMostOffset()].getPtr());
                        }
                    }
                    
                }
                */

                if(!source._sideBack->isLeaf()){
                    throw "break time";
                }

                //* promote position at half-left node
                for(int i = 0; i < _options.track_length; ++i){
                    if(source._data[i].getBitmap(0) && source._data[i].getKey(0) == promote.getKey(0)){
                        std::clog << "<log> <Unit::copyHalfNode()> promoteLeft" << std::endl;

                        if(source._data[i].getSize() == 1){
                            source.deleteData(promote.getKey(0));
                            promoteMid = false;
                            source.connectBackSideUnit((Unit *)source._data[i].getPtr());
                            // throw "test 1";
                        }
                        else{
                            throw "test 2";
                        }
                        //source.connectBackSideUnit((Unit *)destination._data[i].getPtr());
                        //source.connectBackSideUnit((Unit *)source._data[source.getRightMostOffset()].getPtr());
                    }
                }

                //* promote position at half-right node
                for(int i = 0; i < _options.track_length; ++i){
                    if(destination._data[i].getBitmap(0) && destination._data[i].getKey(0) == promote.getKey(0)){
                        std::clog << "<log> <Unit::copyHalfNode()> promoteRight" << std::endl;
                        destination.deleteData(promote.getKey(0));
                        promoteMid = false;
                        source.connectBackSideUnit((Unit *)destination._data[0].getPtr());
                        //source.connectBackSideUnit((Unit *)source._data[source.getRightMostOffset()].getPtr());


                        if(source._sideBack->getValidSize() == 2){
                            std::clog << "<log> <copyHalfNode()> 3. For C" << std::endl;
                            Unit *newUnit = System::allocUnit(_options, source._sideBack->isLeaf());
                            newUnit->setRoot(false);

                            source._sideBack->migrateNode(source._sideBack->_tracks[1], newUnit->_tracks[0], newUnit, 0);
                            //source._sideBack->_tracks[1].setValid(false);

                            destination._data[0].setPtr(newUnit);
                            if(inUnit){
                                newUnit->connectParentUnit((Unit *)promote.getPtr(), 1);
                            }
                            else{
                                newUnit->connectParentUnit((Unit *)promote.getPtr(), 0);
                            }
                            

                            //std::clog << "<log> <copyHalfNode()> newUnit: " << newUnit << std::endl;
                        }
                    }
                }

                //* promote the insert key
                if(promoteMid){
                    std::clog << "<log> <Unit::copyHalfNode()> promoteMid" << std::endl;
                    if(source.getMaxData().getSize() == 2){
                        
                        // throw "test";
                        if(destination._data[0].getSize() == 2){
                            // throw "test";
                        }
                        Offset insertPosition = -1;
                        destination.insertData(source.getMaxIndex(), nullptr, insertPosition, false);
                        source.deleteData(source.getMaxIndex());

                        Unit *newUnit = System::allocUnit(_options, getBackSideUnit()->isLeaf());
                        newUnit->setRoot(false);

                        Offset rightMostOffset = source.getRightMostOffset();
                        Unit *splitUnit = (Unit *)source._data[rightMostOffset].getPtr();
                        if(inUnit){
                            splitUnit->migrateNode(splitUnit->_tracks[1], newUnit->_tracks[0], this, 0);
                        }
                        else{
                            splitUnit->migrateNode(splitUnit->_tracks[1], newUnit->_tracks[0], this, 1);
                        }
                        //splitUnit->_tracks[1].setValid(false);
                        source.connectBackSideUnit(newUnit);
                    }
                    else{
                        // std::clog << "<log> <copyHalfNode()> source._sideBack->isLeaf(): " << source._sideBack->isLeaf() << std::endl;

                        Offset insertPosition = -1;
                        Index minIndex = destination.getMinIndex();
                        destination.deleteData(minIndex);
                        destination.insertData(minIndex, nullptr, insertPosition, false);

                        Unit *newUnit = System::allocUnit(_options, getBackSideUnit()->isLeaf());
                        newUnit->setRoot(false);

                        Unit *splitUnit = (Unit *)destination._data[1].getPtr();
                        if(inUnit){
                            splitUnit->migrateNode(splitUnit->_tracks[0], newUnit->_tracks[0], newUnit, 1);
                            splitUnit->migrateNode(splitUnit->_tracks[1], splitUnit->_tracks[0], splitUnit, 1);
                        }
                        else{
                            splitUnit->migrateNode(splitUnit->_tracks[0], newUnit->_tracks[0], newUnit, 0);
                            splitUnit->migrateNode(splitUnit->_tracks[1], splitUnit->_tracks[0], splitUnit, 0);
                        }

                        source.connectBackSideUnit(newUnit);
                        
                        // throw "test";
                    }
                    /*
                    if(source._sideBack->isLeaf()){
                        std::clog << "<log> <Unit::copyHalfNode()> promoteMid" << std::endl;
                        // throw "test";
                        if(destination._data[0].getSize() == 2){
                            // throw "test";
                        }
                        Offset insertPosition = -1;
                        destination.insertData(source.getMaxIndex(), nullptr, insertPosition, false);
                        source.deleteData(source.getMaxIndex());

                        Unit *newUnit = System::allocUnit(_options, getBackSideUnit()->isLeaf());
                        newUnit->setRoot(false);

                        Offset rightMostOffset = source.getRightMostOffset();
                        Unit *splitUnit = (Unit *)source._data[rightMostOffset].getPtr();
                        if(inUnit){
                            splitUnit->migrateNode(splitUnit->_tracks[1], newUnit->_tracks[0], this, 0);
                        }
                        else{
                            splitUnit->migrateNode(splitUnit->_tracks[1], newUnit->_tracks[0], this, 1);
                        }
                        splitUnit->_tracks[1].setValid(false);
                        source.connectBackSideUnit(newUnit);
                    }
                    else{
                        throw "test mid";
                    }
                    */
                    
                    //source.connectBackSideUnit((Unit *)destination._data[0].getPtr());
                    //source.connectBackSideUnit((Unit *)source._data[source.getRightMostOffset()].getPtr());    
                }
                
                

                if(inUnit){
                    source._sideBack->connectParentUnit(this, 0);
                }
                else{
                    source._sideBack->connectParentUnit(this, 1);
                }

                destination.setValid(true);

                std::clog << "<log> <copyHalfNode()> Internal source end: " << source << std::endl;
                std::clog << "<log> <copyHalfNode()> Internal destination end: " << destination << std::endl;
            }
            
        }
    }
    
    /**
     * * control data deletion
     * TODO evaluation 
     * TODO general unit parameter
     */
    void deleteData(uint64_t idx, uint64_t unit_offset, uint64_t enter_offset, bool &mergeFlag){
        //std::clog << "<log> <deleteData()> begin" << std::endl;
        //std::clog << "<log> <deleteData()> _id: " << _id << std::endl;
        switch (_options.search_mode)
        {
        case Options::search_function::SEQUENTIAL:
            //TODO evaluation 
            break;
        case Options::search_function::TRAD_BINARY_SEARCH:
            //TODO evaluation 
            break;
        case Options::search_function::BIT_BINARY_SEARCH:
            //TODO evaluation 
            break;
        default:
            throw "undefined search operation";
        }

        if(isLeaf()){
            deleteCurrentData(idx, unit_offset, enter_offset, enter_offset, mergeFlag);
        }
        else{
            for(int i = 0; i < _options.unit_size; ++i){
                for(int j = 0; j < _options.track_length; ++j){
                    Unit *nextUnit = (Unit *)_tracks[i]._data[j].getPtr();

                    //TODO
                    if(_tracks[i]._data[j].getBitmap(0) && nextUnit != nullptr && idx < _tracks[i]._data[j].getKey(0)){
                        nextUnit->deleteData(idx, unit_offset, j, mergeFlag);
                        //std::clog << "<log> <deleteData()> mergeFlag: " << mergeFlag << std::endl;
                        //std::clog << "<log> <deleteData()> nextUnit->_tracks[0]: " << nextUnit->_tracks[0] << std::endl;
                        if(mergeFlag){
                            uint64_t deleteIndex = _tracks[i]._data[j].getKey(0);
                            //std::clog << "<log> <deleteData()> deleteIndex: " << deleteIndex << std::endl;
                            //std::clog << "<log> <deleteData()> idx: " << idx << std::endl;
                            deleteCurrentData(deleteIndex, unit_offset, j, enter_offset, mergeFlag);
                        }
                        //adjInternalIndex();
                        return;
                    }
                }
                // check side unit
                Unit *nextUnit = getBackSideUnit();
                nextUnit->deleteData(idx, unit_offset, _options.track_length, mergeFlag);
                //std::clog << "<log> <deleteData()> side mergeFlag: " << mergeFlag << std::endl;
                //std::clog << "<log> <deleteData()> _tracks[unit_offset]._data[_options.track_length-1].getKey(0): " << _tracks[unit_offset]._data[_options.track_length-1].getKey(0) << std::endl;
                //std::clog << "<log> <deleteData()> idx: " << idx << std::endl;

                if(mergeFlag){
                    //std::clog << "<log> <deleteData()> Unit _id: " << _id << std::endl;
                    uint64_t deleteOffset = _tracks[unit_offset].getRightMostOffset();
                    uint64_t deleteIndex = _tracks[unit_offset]._data[deleteOffset].getKey(0);
                    //std::clog << "<log> Side deleteOffset: " << deleteOffset << std::endl;
                    //std::clog << "<log> Side deleteIndex: " << deleteIndex << std::endl;
                    //_tracks[i]._data[_options.track_length-1].setKey(0, nextUnit->_tracks[unit_offset].getMinIndex());
                    deleteCurrentData(deleteIndex, unit_offset, _options.track_length, enter_offset, mergeFlag);
                    //std::clog << "<log> <deleteData()> side nextUnit->_tracks[0]: " << nextUnit->_tracks[0] << std::endl;
                }
                //adjInternalIndex();
                return;
            }

            throw "Delete Fail";
        }
    }

    /**
     * * Delete data at current unit
     * TODO general unit parameter
     * TODO in double tracks, probabaly balance operation
     */
    void deleteCurrentData(uint64_t idx, uint64_t unit_offset, uint64_t data_offset, uint64_t enter_offset, bool &mergeFlag){
        Offset insertPosition = -1;
        
        //std::clog << "<log> <deleteCurrentData()> idx: " << idx << std::endl;
        //std::clog << "<log> <deleteCurrentData()> _id: " << _id << std::endl;
        //std::clog << "<log> <deleteCurrentData()> begin" << std::endl;
        //std::clog << "<log> data_offset: " << data_offset << std::endl;
        //std::clog << "<log> enter_offset: " << enter_offset << std::endl;

        if(_isRoot){
            //std::clog << "<log> delete at root" << std::endl;
            bool dataSide = false;
            if(data_offset == _options.track_length){
                dataSide = true;
            }
            _tracks[unit_offset].deleteData(idx, dataSide);

            if(isEmpty(0)){
                this->setRoot(false);
                if(getBackSideUnit() != nullptr){
                    getBackSideUnit()->setRoot(true);
                    getBackSideUnit()->connectParentUnit(nullptr);
                }
            }
        }
        else if(isLeaf()){
            Unit *selfUnit = this; // to be deleted
            bool side = false;
            Unit *rightUnit = findRightUnit(unit_offset, data_offset, enter_offset);
            Unit *leftUnit = findLeftUnit(unit_offset, data_offset, enter_offset);
            
            //std::clog << "<log> Leaf selfUnit: " << selfUnit << std::endl;
            //std::clog << "<log> Leaf leftUnit: " << leftUnit << std::endl;
            //std::clog << "<log> Leaf rightUnit: " << rightUnit << std::endl;

            // Check merge or borrow
            if(isHalf(unit_offset)){
                if(leftUnit != nullptr){
                    KeyPtrSet borrow = borrowDataFromLeft(*leftUnit, *selfUnit);
                    //std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        //std::clog << "<log> Unit _id: " << _id << std::endl;
                        mergeNodeFromRight(*leftUnit, *selfUnit);
                        mergeFlag = true;
                        leftUnit->_tracks[unit_offset].deleteData(idx); 
                        //std::clog << "<log> test point 1" << std::endl;
                    }
                    else{
                        //getParentUnit()->_tracks[unit_offset]._data[data_offset-1].setKey(0, selfUnit->_tracks[0].getMinIndex());
                        _tracks[unit_offset].deleteData(idx);  
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }

                }
                else if(rightUnit != nullptr){
                    KeyPtrSet borrow = borrowDataFromRight(*selfUnit, *rightUnit);
                    //std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        selfUnit->_tracks[unit_offset].deleteData(idx);
                        mergeNodeFromLeft(*selfUnit, *rightUnit);
                        mergeFlag = true;
                        
                        //std::clog << "<log> rightUnit->_tracks[unit_offset]: " << rightUnit->_tracks[unit_offset] << std::endl;
                    }
                    else{
                        //getParentUnit()->_tracks[unit_offset]._data[data_offset].setKey(0, rightUnit->_tracks[0].getMinIndex());
                        _tracks[unit_offset].deleteData(idx);  
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }
                    //if(data_offset >= 1 && getParentUnit()->_tracks[unit_offset]._data[data_offset-1].getKey(0) == idx){
                    //    getParentUnit()->_tracks[unit_offset]._data[data_offset-1].setKey(0, _tracks[unit_offset].getMinData().getKey(0));
                    //}
                }
                else{
                    _tracks[unit_offset].deleteData(idx);  
                }

                return;
            }
            
            _tracks[unit_offset].deleteData(idx);
        }
        // Else delete at Internal unit
        else
        {
            //std::clog << "<log> <deleteCurrentData() Internal> test point begin" << std::endl;
            Unit *selfUnit = this;
            bool side = false;
            Unit *rightUnit = findRightUnit(unit_offset, data_offset, enter_offset);
            Unit *leftUnit = findLeftUnit(unit_offset, data_offset, enter_offset);
            bool dataSide = false;
            mergeFlag = false;
            //std::clog << "<log> <deleteCurrentData() Internal> test point begin" << std::endl;

            if(data_offset == _options.track_length)
                side = true;

            //std::clog << "<log> Internal selfUnit: " << selfUnit << std::endl;
            //std::clog << "<log> isHalf(unit_offset): " << isHalf(unit_offset) << std::endl;
            //std::clog << "<log> Internal leftUnit: " << leftUnit << std::endl;
            //std::clog << "<log> Internal rightUnit: " << rightUnit << std::endl;
            //std::clog << "<log> side: " << side << std::endl;
            //std::clog << "<log> dataSide: " << dataSide << std::endl;
            
            if(isHalf(unit_offset)){
                if(leftUnit != nullptr){
                    //std::clog << "<log> selfUnit->_tracks[0]: " << selfUnit->_tracks[0] << std::endl;
                    KeyPtrSet borrow = borrowDataFromLeft(*leftUnit, *selfUnit);
                    //std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        //std::clog << "<log> Unit _id: " << _id << std::endl;

                        uint64_t cloestLeftOffset = getParentUnit()->_tracks[0].getClosestLeftOffset(enter_offset);
                        uint64_t idxFromParent = getParentUnit()->_tracks[0]._data[cloestLeftOffset].getKey(0);
                        //
                        selfUnit->_tracks[unit_offset].deleteData(idx, side);
                        leftUnit->_tracks[unit_offset].insertData(idxFromParent, leftUnit->getBackSideUnit(), insertPosition, false);
                        
                        //data_offset = selfUnit->_tracks[unit_offset].getOffsetByIndex(idx);
                        //selfUnit->_tracks[unit_offset]._data[data_offset].setKey(0, idxFromParent);
                        mergeNodeFromRight(*leftUnit, *selfUnit);
                        leftUnit->connectBackSideUnit(selfUnit->getBackSideUnit());
                        mergeFlag = true;
                        
                        //std::clog << "<log> cloestLeftOffset: " << cloestLeftOffset << std::endl; 
                        
                        
                        //std::clog << "<log> idxFromParent: " << idxFromParent << std::endl; 
                        //std::clog << "<log> getParentUnit()->_tracks[0]: " << getParentUnit()->_tracks[0] << std::endl; 
                        if(enter_offset == _options.track_length){
                            //getParentUnit()->_tracks[0]._data[cloestLeftOffset].setKey(0, getParentUnit()->_tracks[0]._data[enter_offset].getKey(0));
                            getParentUnit()->_tracks[0].connectBackSideUnit((Unit *)getParentUnit()->_tracks[0]._data[cloestLeftOffset].getPtr());
                        }
                        else{
                            getParentUnit()->_tracks[0]._data[cloestLeftOffset].setKey(0, getParentUnit()->_tracks[0]._data[enter_offset].getKey(0));
                            getParentUnit()->_tracks[0]._data[enter_offset].setPtr(getParentUnit()->_tracks[0]._data[cloestLeftOffset].getPtr());
                        }
                        //std::clog << "<log> getParentUnit()->_tracks[0]: " << getParentUnit()->_tracks[0] << std::endl; 
                        //std::clog << "<log> idxFromParent: " << idxFromParent << std::endl; 
                        
                        //std::clog << "<log> leftUnit->_tracks[0]: " << leftUnit->_tracks[0] << std::endl;
                        //std::clog << "<log> leftUnit->_tracks[0]: " << leftUnit->_tracks[0] << std::endl;
                    }
                    else{
                        uint64_t cloestLeftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(enter_offset);
                        uint64_t idxFromParent = getParentUnit()->_tracks[unit_offset]._data[cloestLeftOffset].getKey(0);
                        getParentUnit()->_tracks[unit_offset]._data[cloestLeftOffset].setKey(0, borrow.getKey(0));
                        //getParentUnit()->_tracks[unit_offset]._data[data_offset-1].setKey(0, selfUnit->_tracks[0].getMinIndex());
                        uint64_t deleteOffset = _tracks[unit_offset].getOffsetByIndex(idx);
                        _tracks[unit_offset].deleteData(idx, side); //? side 
                        {
                            uint64_t cloestLeftOffset = _tracks[unit_offset].getClosestLeftOffset(deleteOffset);
                            _tracks[unit_offset]._data[cloestLeftOffset].setKey(0, idxFromParent);
                        }
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }
                }
                else if(rightUnit != nullptr){
                    
                    KeyPtrSet borrow = borrowDataFromRight(*selfUnit, *rightUnit);
                    //std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        selfUnit->_tracks[unit_offset].deleteData(idx, side); 
                        mergeNodeFromLeft(*selfUnit, *rightUnit);
                        mergeFlag = true;

                        //uint64_t leftMostOffset = getParentUnit()->_tracks[0].getLeftMostOffset();
                        uint64_t idxFromParent = getParentUnit()->_tracks[0]._data[enter_offset].getKey(0);
                        rightUnit->_tracks[unit_offset].insertData(idxFromParent, selfUnit->getBackSideUnit(), insertPosition, false);
                    }
                    else{
                        getParentUnit()->_tracks[unit_offset]._data[enter_offset].setKey(0, borrow.getKey(0));
                        _tracks[unit_offset].deleteData(idx, side);
                        _tracks[unit_offset].insertData(borrow.getKey(0), borrow.getPtr(), insertPosition);
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }
                }
                else{
                    throw "need to delete but not found neightbor unit";
                }

                //adjInternalIndex();
                //if(leftUnit != nullptr)
                //    leftUnit->adjInternalIndex();
                //if(rightUnit != nullptr)
                //    rightUnit->adjInternalIndex();

                return;
            }
            //if(_tracks[unit_offset].isRightMostOffset(data_offset)){
            //    dataSide = true;
            //}
            _tracks[unit_offset].deleteData(idx, side); 
        }

        //std::clog << "<log> <deleteCurrentData()> end" << std::endl;
        return;
    }

    KeyPtrSet borrowDataFromRight(Unit &left, Unit& right){
        Offset insertPosition = -1;
        //! left_unit_offset = 
        //! right_unit_offset = 
        
        if(right.isLeaf() && !right.isHalf(0)){
            KeyPtrSet borrow = right._tracks[0].getMinData();
            right._tracks[0].deleteData(right._tracks[0].getMinIndex());
            left._tracks[0].insertData(borrow.getKey(0), borrow.getPtr(), insertPosition);
            //std::clog << "<log> destination: " << destination << std::endl;

            return borrow;
        }
        else if(!right.isLeaf() && !right.isHalf(0)){
            KeyPtrSet borrow = right._tracks[0].getMinData();
            right._tracks[0].deleteData(right._tracks[0].getMinIndex());
            ((Unit *)borrow.getPtr())->connectParentUnit(&left);
            //left._tracks[0].insertData(borrow.getKey(0), borrow.getPtr());

            return borrow;
        }

        return KeyPtrSet();
    }

    KeyPtrSet borrowDataFromLeft(Unit &left, Unit &right){
        Offset insertPosition = -1;
        //! left_unit_offset = 
        //! right_unit_offset = 

        if(left.isLeaf() && !left.isHalf(0)){
            KeyPtrSet borrow = left._tracks[0].getMaxData();
            left._tracks[0].deleteData(left._tracks[0].getMaxIndex());
            right._tracks[0].insertData(borrow.getKey(0), borrow.getPtr(), insertPosition);
            return borrow;
        }
        else if(!left.isLeaf() && !left.isHalf(0)){
            KeyPtrSet borrow = left._tracks[0].getMaxData();
            borrow.setPtr(left.getBackSideUnit());
            ((Unit *)borrow.getPtr())->connectParentUnit(&right);
            left._tracks[0].deleteData(left._tracks[0].getMaxIndex(), true);
            right._tracks[0].insertData(borrow.getKey(0), borrow.getPtr(), insertPosition, false);

            return borrow;
        }

        return KeyPtrSet();
    }

    // Merge right Node to left Node
    void mergeNodeFromRight(Unit &left, Unit &right){
        Offset insertPosition = -1;
        //! left_unit_offset = 
        //! right_unit_offset = 

        //std::clog << "<log> <mergeNodeFromRight()> begin left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromRight()> begin right: " << right._tracks[0] << std::endl;

        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
            /* code */
            break;
        case Options::split_merge_function::UNIT:
            /* code */
            break;
        default:
            throw "undefined merge operation";
            break;
        }

        if(left.isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                if(right._tracks[0]._data[i].getBitmap(0)){ //TODO
                    left._tracks[0].insertData(right._tracks[0]._data[i].getKey(0), right._tracks[0]._data[i].getPtr(), insertPosition);
                    right._tracks[0].deleteMark(i);
                }
            }
        }
        else{
            Unit *tempSide = left.getBackSideUnit();
            left.connectBackSideUnit(nullptr);
            bool first = true;
            for(int i = 0; i < _options.track_length; ++i){
                if(right._tracks[0]._data[i].getBitmap(0)){ //TODO
                    ((Unit *)right._tracks[0]._data[i].getPtr())->connectParentUnit(&left);
                    left._tracks[0].insertData(right._tracks[0]._data[i].getKey(0), right._tracks[0]._data[i].getPtr(), insertPosition, false);
                    right._tracks[0].deleteMark(i);
                }
            }
            right.getBackSideUnit()->connectParentUnit(&left);
            left.connectBackSideUnit(tempSide);
        }

        //std::clog << "<log> <mergeNodeFromRight()> end left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromRight()> end right: " << right._tracks[0] << std::endl;
    }

    void mergeNodeFromLeft(Unit &left, Unit &right){
        Offset insertPosition = -1;
        //! left_unit_offset = 
        //! right_unit_offset = 
        //std::clog << "<log> <mergeNodeFromLeft()> begin left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromLeft()> begin right: " << right._tracks[0] << std::endl;
        
        if(left.isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                if(left._tracks[0]._data[i].getBitmap(0)){ //TODO
                    right._tracks[0].insertData(left._tracks[0]._data[i].getKey(0), left._tracks[0]._data[i].getPtr(), insertPosition);
                    left._tracks[0].deleteMark(i);
                }
            }
            //std::clog << "<log> left: " << left << std::endl;
            //std::clog << "<log> right: " << right << std::endl;
        }
        else{
            //left.connectSideUnit(right._side);
            for(int i = 0; i < _options.track_length; ++i){
                if(left._tracks[0]._data[i].getBitmap(0)){ //TODO
                    ((Unit *)left._tracks[0]._data[i].getPtr())->connectParentUnit(&right);
                    right._tracks[0].insertData(left._tracks[0]._data[i].getKey(0), left._tracks[0]._data[i].getPtr(), insertPosition, false);
                    left._tracks[0].deleteMark(i);
                }
            }
            left.getBackSideUnit()->connectParentUnit(&right);
            //right._tracks[0].insertData(left.getSideUnit()->_tracks[0].getMinIndex(), left.getSideUnit(), false);
            //std::clog << "<log> left: " << left << std::endl;
            //std::clog << "<log> right: " << right << std::endl;
        }

        //std::clog << "<log> <mergeNodeFromLeft()> end left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromLeft()> end right: " << right._tracks[0] << std::endl;
    }

    /**
     * * for double tracks
     * @param left is non-full node
     * @param right is full node
    */
    void balanceDataFromRight(Node &left, Node &right){
        Offset insertPosition = -1;

        if(isLeaf()){
            if(right.getSize() - left.getSize() == 1){
                left.insertData(right._data[0].getKey(0), right._data[0].getPtr(), insertPosition);
                right.deleteMark(0);
            }
            else{
                for(int i = 0; i < (right.getSize() - left.getSize()) / 2 ; ++i){
                    left.insertData(right._data[i].getKey(0), right._data[i].getPtr(), insertPosition);
                    right.deleteData(right._data[i].getKey(0));
                }
            }
        }
        else{
            if(right.getSize() - left.getSize() == 1){
                ((Unit *)right._data[0].getPtr())->connectParentUnit(this, 0);
                left.insertDataFromBack(right._data[0]);

                uint64_t rightMostoffset = left.getRightMostOffset();
                void *temp = left._sideBack;
                left.connectBackSideUnit((Unit *)left._data[rightMostoffset].getPtr());
                left._data[rightMostoffset].setPtr(temp);

                right.deleteMark(0);
            }
            else{
                for(int i = 0; i < (right.getSize() - left.getSize()) / 2; ++i){
                    ((Unit *)right._data[i].getPtr())->connectParentUnit(this, 0);
                    left.insertDataFromBack(right._data[i]);

                    uint64_t rightMostoffset = left.getRightMostOffset();
                    void *temp = left._sideBack;
                    left.connectBackSideUnit((Unit *)left._data[rightMostoffset].getPtr());
                    left._data[rightMostoffset].setPtr(temp);

                    right.deleteMark(i);
                }

                std::clog << "<log> <balanceDataFromRight()> Internal left: " << left << std::endl;
                std::clog << "<log> <balanceDataFromRight()> Internal right: " << right << std::endl;
            }
        }

        
    }

    /**
     * * for double tracks
     * @param left is full node
     * @param right is non-full node
    */
    void balanceDataFromLeft(Node &left, Node &right, KeyPtrSet promote = KeyPtrSet()){
        Offset insertPosition = -1;
        uint64_t begin = (left.getSize() + right.getSize()) / 2;

        if(isLeaf()){
            for(int i = begin; i < _options.track_length; ++i){
                right.insertData(left._data[i].getKey(0), left._data[i].getPtr(), insertPosition);
                left.deleteData(left._data[i].getKey(0));
            }

            //std::clog << "<log> <balanceDataFromLeft()> Leaf left: " << left << std::endl;
            //std::clog << "<log> <balanceDataFromLeft()> Leaf right: " << right << std::endl;
        }
        else{
            /*
            left._sideBack->connectParentUnit(this, 1);
            right.connectBackSideUnit(left._sideBack);

            for(int i = begin; i < _options.track_length; ++i){
                ((Unit *)left._data[i].getPtr())->connectParentUnit(this, 1);
                right.insertDataFromBack(left._data[i]);
                left.deleteMark(i);
            }
            uint64_t rightMostOffset = left.getRightMostOffset();
            left.connectBackSideUnit((Unit *)left._data[rightMostOffset].getPtr());
            */

            uint64_t rightMostOffset = left.getRightMostOffset();
            uint64_t maxIndex = left.getMaxIndex();

            if(left._sideBack->getValidSize() == 1){
                left._sideBack->connectParentUnit(this, 1);
                right.insertData(maxIndex, left._sideBack, insertPosition, false);
                left.deleteData(maxIndex);
                left.connectBackSideUnit((Unit *)left._data[left.getRightMostOffset()].getPtr());
            }
            else if(left._sideBack->getParentOffset(1) != left._sideBack->getParentOffset(0)){
                left._sideBack->connectParentUnit(this, 1);
                right.insertData(maxIndex, left._sideBack, insertPosition, false);
                left.deleteData(maxIndex);
                left.connectBackSideUnit((Unit *)left._data[left.getRightMostOffset()].getPtr());
            }
            else{
                left._sideBack->connectParentUnit(this, 1, 1);
                right.insertData(maxIndex, left._sideBack, insertPosition, false);
                left.deleteData(maxIndex);
            }

            std::clog << "<log> <balanceDataFromLeft()> Internal left: " << left << std::endl;
            std::clog << "<log> <balanceDataFromLeft()> Internal right: " << right << std::endl;
        }

        
    }

    /**
     * * for double tracks
     * * From left to right
     * @param left
     * @param right
    */
    void migrateNode(Node &left, Node &right, Unit *parentUnit = nullptr, unsigned parentUnitOffset = 0){
        if(left._isLeaf != right._isLeaf){
            throw "<migrateNode()> type error";
        }

        std::clog << "<log> <Unit::migrateNode()>" << std::endl;
        std::clog << "<log> <migrateNode() begin> left: " << left << std::endl;
        std::clog << "<log> <migrateNode() begin> right: " << right << std::endl;

        if(left._isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(left._data[i].getBitmap(0)){
                    right._data[i] = left._data[i];
                    left.deleteMark(i);
                }
            }
        }
        else{
            if(parentUnit == nullptr){
                throw "<Unit::migrateNode()::Internal> error";
            }

            for(int i = 0; i < _options.track_length; ++i){
                if(left._data[i].getBitmap(0)){
                    right._data[i] = left._data[i];
                    if((Unit *)right._data[i].getPtr() != nullptr)
                        ((Unit *)right._data[i].getPtr())->connectParentUnit(parentUnit, parentUnitOffset);
                    left.deleteMark(i);
                }
            }
            right.connectBackSideUnit(left._sideBack);
            right._sideBack->connectParentUnit(parentUnit, parentUnitOffset);
        }

        left.setValid(false);
        right.setValid(true);

        std::clog << "<log> <migrateNode() end> left: " << left << std::endl;
        std::clog << "<log> <migrateNode() end> right: " << right << std::endl;
    }

    /* Minor Functions */

    Unit *findRightUnit(uint64_t unit_offset, uint64_t data_offset, uint64_t enter_offset){
        //std::clog << "<log> enter_offset: " << enter_offset << std::endl;
        //std::clog << "<log> data_offset: " << data_offset << std::endl;
        
        if(_isRoot)
            return nullptr;

        if(enter_offset == _options.track_length){
            return nullptr;
        }

        if(getParentUnit()->_tracks[unit_offset].isRightMostOffset(enter_offset)){
            return getParentUnit()->getBackSideUnit();
        }

        if(!getParentUnit()->_tracks[unit_offset].isRightMostOffset(enter_offset)){
            uint64_t rightOffset = getParentUnit()->_tracks[unit_offset].getClosestRightOffset(enter_offset);
            return (Unit *)getParentUnit()->_tracks[unit_offset]._data[rightOffset].getPtr();
            //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
        }

        return nullptr;
    }

    Unit *findLeftUnit(uint64_t unit_offset, uint64_t data_offset, uint64_t enter_offset){
        if(_isRoot)
            return nullptr;
        
        if(!getParentUnit()->_tracks[unit_offset].isLeftMostOffset(enter_offset)){
            uint64_t leftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(enter_offset);
            return (Unit *)getParentUnit()->_tracks[unit_offset]._data[leftOffset].getPtr();
            //std::clog << "<log> leftUnit: " << leftUnit << std::endl;
        }

        return nullptr;
    }

    /**
     * * adjust internal unit index
     * ! most inefficient
     * TODO general unit parameter
     * TODO make it just checking current unit/node
    */
    void adjInternalIndex(uint64_t unit_offset = 0){
        //std::clog << "<log> <adjInternalIndex()> begin" << std::endl;
        if(!isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(_tracks[unit_offset]._data[i].getBitmap(j)){ //TODO
                        ((Unit *)_tracks[unit_offset]._data[i].getPtr())->adjInternalIndex(j);

                        Unit *nextUnit = nullptr;
                        //std::clog << "<log> <adjInternalIndex()> nextUnit: " << nextUnit << std::endl;
                        if(_tracks[unit_offset].isRightMostOffset(i)){
                            nextUnit = getBackSideUnit(unit_offset);
                        }
                        else{
                            uint64_t cloestRightOffset = _tracks[unit_offset].getClosestRightOffset(i);
                            nextUnit = (Unit *)_tracks[unit_offset]._data[cloestRightOffset / _options.unit_size].getPtr();
                        }
                        //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0]: " << nextUnit->_tracks[0] << std::endl;
                        //std::clog << "<log> <adjInternalIndex()> nextUnit: " << nextUnit << std::endl;

                        if(nextUnit->isLeaf()){
                            _tracks[unit_offset]._data[i].setKey(j, nextUnit->_tracks[j].getMinIndex());
                            //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0].getMinIndex(): " << nextUnit->_tracks[0].getMinIndex() << std::endl;
                        }
                        else{
                            while(!nextUnit->isLeaf()){
                                //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0] ori: " << nextUnit->_tracks[0] << std::endl;
                                uint64_t leftMostOffset = nextUnit->_tracks[j].getLeftMostOffset();
                                nextUnit = (Unit *)nextUnit->_tracks[j]._data[leftMostOffset / _options.unit_size].getPtr();
                                //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0] next: " << nextUnit->_tracks[0] << std::endl;
                            }
                            //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0].getMinIndex(): " << nextUnit->_tracks[0].getMinIndex() << std::endl;
                            _tracks[unit_offset]._data[i].setKey(j, nextUnit->_tracks[j].getMinIndex());
                        }
                        
                    }
                }
                
            }
            getBackSideUnit()->adjInternalIndex();
        }
    }

    /* Supported Functions */

    bool isFull(uint64_t offset) const{
        return _tracks[offset].isFull();
    }

    bool isFullUnit() const{
        //std::clog << "<log> <isFullUnit()> begin" << std::endl;
        for(int i = 0; i < _options.unit_size; ++i){
            if(!isValid(i) || !isFull(i)){
                return false;
            }
        }

        return true;
    }

    // For root unit to check empty
    bool isEmpty(uint64_t offset){
        for(int i = 0; i < _options.track_length; ++i){
            if(_tracks[offset]._data[i].getBitmap(0)){ //TODO
                return false;
            }
        }
        return true;
    }

    bool isHalf(uint64_t offset) const{
        return _tracks[offset].isHalf();
    }

    bool isLeaf() const{
        return _tracks[0]._isLeaf;
    }

    bool isValid(uint64_t offset) const{
        return _tracks[offset].isValid();
    }

    bool isAllValid() const{
        //std::clog << "<log> <isAllValid()> begin" << std::endl;

        for(int i = 0; i < _options.unit_size; ++i){
            if(!_tracks[i].isValid()){
                //std::clog << "<log> <isAllValid()> end" << std::endl;
                return false;
            }
        }
        //std::clog << "<log> <isAllValid()> end" << std::endl;
        return true;
    }

    bool isRoot() const{
        return _isRoot;
    }

    /**
     * * For double track
     * * Check that it is possible to insert if have same unit pointer
     * ! Not a good function
    */
    bool isPossibleInsert(uint64_t unit_offset, void *wait_insert_pointer){
        for(int i = 0; i < _options.track_length; ++i){
            if(_tracks[unit_offset]._data[i].getBitmap(0) && _tracks[unit_offset]._data[i].getSize() < _options.unit_size && _tracks[unit_offset]._data[i].getPtr() == wait_insert_pointer){
                return true;
            }
        }

        return false;
    }

    void setLeaf(bool isLeaf){
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i]._isLeaf = isLeaf;
        }
    }

    uint64_t getDataSize(uint64_t offset) const{
        return _tracks[offset].getSize();
    }

    uint64_t getValidSize() const{
        uint64_t counter = 0;
        for(int i = 0; i < _options.unit_size; ++i){
            if(_tracks[i]._isValid){
                ++counter;
            }
        }

        return counter;
    }

    Unit *getRoot(){
        Unit *unit = this;
        while(unit->_tracks[0]._parent != nullptr){
            unit = unit->_tracks[0]._parent;
        }

        return unit;
    }

    /* Data Member */

    Node *_tracks;
    
    /* System */

    void setRoot(bool status){
        _isRoot = status;
    }

    void sideChecker(){

    }

    bool _isRoot;
    Options _options;
    uint64_t _id;
};

#include<unordered_map>
namespace System{
    // store unit pointer
    // store how many the tracks of the unit is using
    //std::unordered_map<Unit *, uint64_t> unitPool;
    
    Unit *allocUnit(Options options, bool isLeaf){
        Unit *newUnit = nullptr;

        switch (options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
            // return unit, which is not all using (empty or partial)
            // for traditional algorithm on multiple tracks
            /*
            for(auto &it: unitPool){
                if(it.second < options.unit_size){
                    newUnit = it.first;
                    unitPool[newUnit] += 1;
                    break;
                }
            }*/

            break;
        case Options::split_merge_function::UNIT:
            // return unused unit (all tracks are not using)

            break;
        default:
            break;
        }

        // allocate new unit
        if(newUnit == nullptr){
            newUnit = new Unit(options, isLeaf);
            //unitPool[newUnit] = 1;
        }
        
        return newUnit;
    }
}

std::ostream &operator<<(std::ostream &out, const Unit &right){
    //std::clog << "<log> Unit " << right._id << " Print" << std::endl;
    //* status
    // out << "Unit " << right._id << "\n";

    //* debug
    out << "\t[\t\n";
    out << "\t\t";
    if(right.isLeaf()) out << "Leaf Unit";
    else out << "Internal Unit";
    out << " " << right._id << " " << &right << "\n";
    if(right._options.unit_size > 1){
        out << "\t\t" << "Left Parent: " << right.getParentUnit(0) << " Offset " << right.getParentOffset(0)  << "\n";
        out << "\t\t" << "Right Parent: " << right.getParentUnit(1) << " Offset " << right.getParentOffset(1)  << "\n";
    }
    else{
        out << "\t\t" << "Parent: " << right.getParentUnit() << " Offset " << right.getParentOffset()  << "\n";
    }
    out << "\t\t";
    if(right._isRoot) out << "Root" << "\n";

    bool first = true;
    for(int i = 0; i < right._options.unit_size; ++i){
        if(first)first = false;
        else out << ", \n";
        out << right._tracks[i];
    }
    out << "\n\t]\n";
    if(!right.isLeaf()){
        for(int i = 0; i < right._options.unit_size; ++i){
            for(int j = 0; j < right._options.track_length; ++j){
                if(right._tracks[i]._data[j].getBitmap(0) && right._tracks[i]._data[j].getPtr() != nullptr){ //TODO
                    out << *(Unit *)right._tracks[i]._data[j].getPtr();
                }
            }

            uint64_t rightMostOffset = right._tracks[i].getRightMostOffset();
            if(right._tracks[i]._sideBackBitmap && right._tracks[i]._sideBack != nullptr && right._tracks[i]._data[rightMostOffset].getPtr() != right._tracks[i]._sideBack){
                out << *(Unit *)right._tracks[i]._sideBack;
            }
        }
    }

    return out;
}

#endif