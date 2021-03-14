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

    void updateData(unsigned idx, unsigned data, unsigned offset){

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
                setRoot(false);

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

            getRoot()->insertData(idx, data, 0);
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
                setRoot(false);

                Unit *newRoot = new Unit(_options);
                newRoot->deLeaf();

                newRoot->insertCurrentPointer(*promote.key, this, 0);
                this->connectParentUnit(newRoot);

                newRoot->connectSideUnit((Unit *)promote.ptr);
                ((Unit *)promote.ptr)->connectParentUnit(newRoot);
                
                Unit *rightUnit = (Unit *)promote.ptr;
                //std::clog << "<log> idx: " << idx << std::endl;
                if(idx < promote.getKey(0)){
                    insertCurrentPointer(idx, unit, offset);
                    unit->connectParentUnit(this);
                }
                else if(idx > promote.getKey(0)){
                    rightUnit->insertCurrentPointer(idx, unit, 0);
                    unit->connectParentUnit(rightUnit);
                }
                else{ 
                    unsigned leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                    rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                    unit->connectParentUnit(rightUnit);
                }
                //std::clog << "<log> newRoot: " << (*newRoot)._tracks[0] << std::endl;
                return;
            }
            //insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
            //((Unit *)promote.ptr)->connectParentUnit(this, 0);
            this->_tracks[0]._parent->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
            if(((Unit *)promote.ptr)->getParentUnit() == nullptr)
                ((Unit *)promote.ptr)->connectParentUnit(getParentUnit());


            //Unit *rightUnit = getParentRightUnit(idx);
            Unit *rightUnit = (Unit *)promote.ptr;
            if(idx < promote.getKey(0)){
                insertCurrentPointer(idx, unit, offset);
                unit->connectParentUnit(this);
            }
            else if(idx > promote.getKey(0)){
                rightUnit->insertCurrentPointer(idx, unit, 0);
                unit->connectParentUnit(rightUnit);
            }
            else{
                unsigned leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                //((Unit *)rightUnit->_tracks[0]._data[0].getPtr())->connectParentUnit(rightUnit);
                unit->connectParentUnit(rightUnit);
                //std::clog << "<log> rightUnit->_tracks[0]._data[0].getPtr(): " << rightUnit->_tracks[0]._data[0].getPtr() << std::endl;
                //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
                //((Unit *)promote.getPtr())->connectParentUnit(getParentRightUnit());
            }

            return;
        }

        _tracks[offset].insertData(idx, unit);
        unit->connectParentUnit(this);
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

    

    // Return KeyPtrSet
    KeyPtrSet splitNode(unsigned wait_insert_idx, unsigned offset){
        KeyPtrSet promote(2);

        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
        {
            Unit *newUnit = new Unit(_options);
            newUnit->setRoot(false);
            if(!isLeaf()) newUnit->deLeaf();

            unsigned promoteKey = System::getMid(_tracks[offset]._data, _options.track_length, wait_insert_idx);
            promote.setPtr(newUnit);
            promote.addKey(promoteKey);
            
            copyHalfNode(_tracks[offset], newUnit->_tracks[0], promote, wait_insert_idx);
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

    void copyHalfNode(Node &source, Node &destination, KeyPtrSet promote, unsigned wait_insert_idx){
        if(isLeaf()){
            switch(_options.node_ordering){
                case Options::ordering::SORTED:
                    if(wait_insert_idx < promote.getKey(0)){
                        for(int i = _options.track_length / 2 - 1, j = 0; i < _options.track_length; ++i, ++j){
                            destination._data[j] = source._data[i];

                            destination._bitmap[j] = true;
                            source._bitmap[i] = false;
                        }
                    }
                    else{
                        for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                            destination._data[j] = source._data[i];

                            destination._bitmap[j] = true;
                            source._bitmap[i] = false;
                        }
                    }
                    break;
                case Options::ordering::UNSORTED:
                    break;
                default:
                    throw "undefined ordering";
            }
        }
        else{
            unsigned mid = _options.track_length / 2;
            if(wait_insert_idx < promote.getKey(0)){
                for(int i = _options.track_length / 2 - 1, j = 0; i < _options.track_length; ++i, ++j){
                    destination._data[j] = source._data[i];

                    destination._bitmap[j] = true;
                    source._bitmap[i] = false;

                    ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr());
                }
            }
            else{
                for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                    destination._data[j] = source._data[i];

                    destination._bitmap[j] = true;
                    source._bitmap[i] = false;

                    ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr());
                }
            }
            destination.connectSideUnit(source._side);

            bool promoteMid = true;
            for(int i = 0; i < _options.track_length; ++i){
                if(source._data[i].getKey(0) == promote.getKey(0)){
                    source.deleteMark(i);
                    promoteMid = false;
                    source.connectSideUnit((Unit *)source._data[i].getPtr());
                }
            }
            for(int i = 0; i < _options.track_length; ++i){
                if(destination._data[i].getKey(0) == promote.getKey(0)){
                    destination.deleteMark(i);
                    promoteMid = false;
                    source.connectSideUnit((Unit *)destination._data[0].getPtr());
                }
            }
            //std::clog << "<log> destination: " << destination << std::endl;

            if(promoteMid){
                source.connectSideUnit((Unit *)destination._data[0].getPtr());
                //destination._data[0].setPtr((Unit *)promote.getPtr());
            }
        }
    }

    void deleteData(unsigned idx, unsigned unit_offset, unsigned data_offset, bool &mergeFlag){
        //std::clog << "<log> <deleteData()> begin" << std::endl;
        //std::clog << "<log> <deleteData()> _id: " << _id << std::endl;

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

        if(isLeaf()){
            deleteCurrentData(idx, unit_offset, data_offset, data_offset, mergeFlag);
            std::clog << "<log> <deleteData()> mergeFlag: " << mergeFlag << std::endl;
            std::clog << "<log> <deleteData()> end" << std::endl;
        }
        else{
            for(int i = 0; i < _options.unit_size; ++i){
                for(int j = 0; j < _options.track_length; ++j){
                    Unit *nextUnit = (Unit *)_tracks[i]._data[j].getPtr();

                    if(_tracks[i]._bitmap[j] && nextUnit != nullptr && idx < _tracks[i]._data[j].getKey(0)){
                        nextUnit->deleteData(idx, unit_offset, j, mergeFlag);
                        std::clog << "<log> <deleteData()> mergeFlag: " << mergeFlag << std::endl;
                        std::clog << "<log> <deleteData()> nextUnit->_tracks[0]: " << nextUnit->_tracks[0] << std::endl;
                        if(mergeFlag){
                            
                            unsigned deleteIndex = _tracks[i]._data[j].getKey(0);
                            //std::clog << "<log> <deleteData()> deleteIndex: " << deleteIndex << std::endl;
                            //std::clog << "<log> <deleteData()> idx: " << idx << std::endl;
                            deleteCurrentData(deleteIndex, unit_offset, j, data_offset, mergeFlag);

                            //std::clog << "<log> <deleteData()> test point begin" << std::endl;
                            /*
                            std::clog << "<log> <deleteData()> nextUnit->isLeaf(): " << nextUnit->isLeaf() << std::endl;
                            std::clog << "<log> <deleteData()> data_offset: " << data_offset << std::endl;
                            if(nextUnit->isLeaf()){
                                bool side = false;
                                
                                Unit *rightUnit = findRightUnit(unit_offset, j, data_offset, side);
                                Unit *leftUnit = findLeftUnit(unit_offset, j, data_offset);

                                std::clog << "<log> <deleteData()> _id: " << _id << std::endl;
                                std::clog << "<log> <deleteData()> leftUnit: " << leftUnit << std::endl;
                                std::clog << "<log> <deleteData()> rightUnit: " << rightUnit << std::endl;

                                for(int k = 0; k < _options.track_length; ++k){
                                    if(_tracks[i]._bitmap[k]){
                                        if(_tracks[i].isRightMostOffset(k) || k == _options.track_length-1){
                                            _tracks[i]._data[k].setKey(0, getSideUnit()->_tracks[0].getMinIndex());
                                        }
                                        else if(_tracks[i].getRightMostOffset() != -1){
                                            unsigned cloestRightOffset = _tracks[i].getClosestRightOffset(k);
                                            Unit *next = (Unit *)_tracks[i]._data[cloestRightOffset].getPtr();
                                            _tracks[i]._data[k].setKey(0, next->_tracks[0].getMinIndex());
                                        }
                                    }
                                }

                                if(leftUnit != nullptr){
                                    //std::clog << "<log> <deleteData()> _tracks[i]: " << leftUnit->_tracks[i] << std::endl;
                                    for(int k = 0; k < _options.track_length; ++k){
                                        if(leftUnit->_tracks[i]._bitmap[k]){
                                            if(leftUnit->_tracks[i].isRightMostOffset(k) || k == _options.track_length-1){
                                                leftUnit->_tracks[i]._data[k].setKey(0, leftUnit->getSideUnit()->_tracks[0].getMinIndex());
                                            }
                                            else if(leftUnit->_tracks[i].getRightMostOffset() != -1){
                                                unsigned cloestRightOffset = leftUnit->_tracks[i].getClosestRightOffset(k);
                                                Unit *next = (Unit *)leftUnit->_tracks[i]._data[cloestRightOffset].getPtr();
                                                leftUnit->_tracks[i]._data[k].setKey(0, next->_tracks[0].getMinIndex());
                                            }
                                        }
                                    }
                                    //std::clog << "<log> <deleteData()> _tracks[i]: " << leftUnit->_tracks[i] << std::endl;
                                }

                                if(rightUnit != nullptr){
                                    //std::clog << "<log> <deleteData()> _tracks[i]: " << rightUnit->_tracks[i] << std::endl;
                                    for(int k = 0; k < _options.track_length; ++k){
                                        if(rightUnit->_tracks[i]._bitmap[k]){
                                            if(rightUnit->_tracks[i].isRightMostOffset(k) || k == _options.track_length-1){
                                                rightUnit->_tracks[i]._data[k].setKey(0, rightUnit->getSideUnit()->_tracks[0].getMinIndex());
                                            }
                                            else if(rightUnit->_tracks[i].getRightMostOffset() != -1){
                                                unsigned cloestRightOffset = rightUnit->_tracks[i].getClosestRightOffset(k);
                                                Unit *next = (Unit *)rightUnit->_tracks[i]._data[cloestRightOffset].getPtr();
                                                rightUnit->_tracks[i]._data[k].setKey(0, next->_tracks[0].getMinIndex());
                                            }
                                        }
                                    }
                                    //std::clog << "<log> <deleteData()> _tracks[i]: " << rightUnit->_tracks[i] << std::endl;
                                }
                                
                            }
                            */
                            //std::clog << "<log> <deleteData()> test point end" << std::endl;
                            
                        }
                        adjInternalIndex();
                        return;
                    }
                }
                // check side unit
                Unit *nextUnit = getSideUnit();
                nextUnit->deleteData(idx, unit_offset, _options.track_length, mergeFlag);
                std::clog << "<log> <deleteData()> side mergeFlag: " << mergeFlag << std::endl;
                
                        
                //std::clog << "<log> <deleteData()> _tracks[unit_offset]._data[_options.track_length-1].getKey(0): " << _tracks[unit_offset]._data[_options.track_length-1].getKey(0) << std::endl;
                //std::clog << "<log> <deleteData()> idx: " << idx << std::endl;

                if(mergeFlag){
                    //std::clog << "<log> <deleteData()> Unit _id: " << _id << std::endl;
                    unsigned deleteOffset = _tracks[unit_offset].getRightMostOffset();
                    unsigned deleteIndex = _tracks[unit_offset]._data[deleteOffset].getKey(0);
                    //std::clog << "<log> Side deleteOffset: " << deleteOffset << std::endl;
                    //std::clog << "<log> Side deleteIndex: " << deleteIndex << std::endl;
                    //_tracks[i]._data[_options.track_length-1].setKey(0, nextUnit->_tracks[unit_offset].getMinIndex());
                    deleteCurrentData(deleteIndex, unit_offset, _options.track_length, data_offset, mergeFlag);
                    std::clog << "<log> <deleteData()> side nextUnit->_tracks[0]: " << nextUnit->_tracks[0] << std::endl;
                    /*
                    if(nextUnit->isLeaf()){
                        //std::clog << "<log> data_offset: " << data_offset << std::endl;
                        //std::clog << "<log> <deleteData()> _id: " << _id << std::endl;

                        for(int k = 0; k < _options.track_length; ++k){
                            if(_tracks[i]._bitmap[k]){
                                if(_tracks[i].isRightMostOffset(k) || k == _options.track_length-1){
                                    _tracks[i]._data[k].setKey(0, getSideUnit()->_tracks[0].getMinIndex());
                                }
                                else if(_tracks[i].getRightMostOffset() != -1){
                                    unsigned cloestRightOffset = _tracks[i].getClosestRightOffset(k);
                                    Unit *next = (Unit *)_tracks[i]._data[cloestRightOffset].getPtr();
                                    _tracks[i]._data[k].setKey(0, next->_tracks[0].getMinIndex());
                                }
                            }
                        }

                        if(getParentUnit() != nullptr){
                            Unit *leftUnit = findLeftUnit(unit_offset, _options.track_length, data_offset);
                            std::clog << "<log> leftUnit: " << leftUnit << std::endl;
                            if(leftUnit != nullptr){
                                //std::clog << "<log> <deleteData()> _tracks[i]: " << leftUnit->_tracks[i] << std::endl;
                                for(int k = 0; k < _options.track_length; ++k){
                                    if(leftUnit->_tracks[i]._bitmap[k]){
                                        if(leftUnit->_tracks[i].isRightMostOffset(k) || k == _options.track_length-1){
                                            leftUnit->_tracks[i]._data[k].setKey(0, leftUnit->getSideUnit()->_tracks[0].getMinIndex());
                                        }
                                        else if(leftUnit->_tracks[i].getRightMostOffset() != -1){
                                            unsigned cloestRightOffset = leftUnit->_tracks[i].getClosestRightOffset(k);
                                            Unit *next = (Unit *)leftUnit->_tracks[i]._data[cloestRightOffset].getPtr();
                                            leftUnit->_tracks[i]._data[k].setKey(0, next->_tracks[0].getMinIndex());
                                        }
                                    }
                                }
                                //std::clog << "<log> <deleteData()> _tracks[i]: " << leftUnit->_tracks[i] << std::endl;
                            }
                        }
                        
                    }*/
                }
                adjInternalIndex();
                return;
            }

            throw "Delete Fail";
        }
    }

    void deleteCurrentData(unsigned idx, unsigned unit_offset, unsigned data_offset, unsigned enter_offset, bool &mergeFlag){
        std::clog << "<log> <deleteCurrentData()> idx: " << idx << std::endl;
        std::clog << "<log> <deleteCurrentData()> _id: " << _id << std::endl;
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
                if(getSideUnit() != nullptr){
                    getSideUnit()->setRoot(true);
                    getSideUnit()->connectParentUnit(nullptr);
                }
            }
        }
        else if(isLeaf()){
            Unit *selfUnit = this; // to be delete
            bool side = false;
            Unit *rightUnit = findRightUnit(unit_offset, data_offset, enter_offset, side);
            Unit *leftUnit = findLeftUnit(unit_offset, data_offset, enter_offset);

            

            std::clog << "<log> Leaf selfUnit: " << selfUnit << std::endl;
            std::clog << "<log> Leaf leftUnit: " << leftUnit << std::endl;
            std::clog << "<log> Leaf rightUnit: " << rightUnit << std::endl;

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
            Unit *rightUnit = findRightUnit(unit_offset, data_offset, enter_offset, side);
            Unit *leftUnit = findLeftUnit(unit_offset, data_offset, enter_offset);
            bool dataSide = false;
            mergeFlag = false;
            //std::clog << "<log> <deleteCurrentData() Internal> test point begin" << std::endl;

            if(data_offset == _options.track_length)
                side = true;

            std::clog << "<log> selfUnit: " << selfUnit << std::endl;
            //std::clog << "<log> isHalf(unit_offset): " << isHalf(unit_offset) << std::endl;
            std::clog << "<log> leftUnit: " << leftUnit << std::endl;
            std::clog << "<log> rightUnit: " << rightUnit << std::endl;
            std::clog << "<log> side: " << side << std::endl;
            //std::clog << "<log> dataSide: " << dataSide << std::endl;

            
            if(isHalf(unit_offset)){
                if(leftUnit != nullptr){
                    //std::clog << "<log> selfUnit->_tracks[0]: " << selfUnit->_tracks[0] << std::endl;
                    KeyPtrSet borrow = borrowDataFromLeft(*leftUnit, *selfUnit);
                    //std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        //std::clog << "<log> Unit _id: " << _id << std::endl;

                        unsigned cloestLeftOffset = getParentUnit()->_tracks[0].getClosestLeftOffset(enter_offset);
                        unsigned idxFromParent = getParentUnit()->_tracks[0]._data[cloestLeftOffset].getKey(0);
                        //
                        selfUnit->_tracks[unit_offset].deleteData(idx, side);
                        leftUnit->_tracks[unit_offset].insertData(idxFromParent, leftUnit->getSideUnit(), false);
                        
                        //data_offset = selfUnit->_tracks[unit_offset].getOffsetByIndex(idx);
                        //selfUnit->_tracks[unit_offset]._data[data_offset].setKey(0, idxFromParent);
                        mergeNodeFromRight(*leftUnit, *selfUnit);
                        leftUnit->connectSideUnit(selfUnit->getSideUnit());
                        mergeFlag = true;
                        
                        //std::clog << "<log> cloestLeftOffset: " << cloestLeftOffset << std::endl; 
                        
                        
                        //std::clog << "<log> idxFromParent: " << idxFromParent << std::endl; 
                        std::clog << "<log> getParentUnit()->_tracks[0]: " << getParentUnit()->_tracks[0] << std::endl; 
                        if(enter_offset == _options.track_length){
                            //getParentUnit()->_tracks[0]._data[cloestLeftOffset].setKey(0, getParentUnit()->_tracks[0]._data[enter_offset].getKey(0));
                            getParentUnit()->_tracks[0].connectSideUnit((Unit *)getParentUnit()->_tracks[0]._data[cloestLeftOffset].getPtr());
                        }
                        else{
                            getParentUnit()->_tracks[0]._data[cloestLeftOffset].setKey(0, getParentUnit()->_tracks[0]._data[enter_offset].getKey(0));
                            getParentUnit()->_tracks[0]._data[enter_offset].setPtr(getParentUnit()->_tracks[0]._data[cloestLeftOffset].getPtr());
                        }
                        std::clog << "<log> getParentUnit()->_tracks[0]: " << getParentUnit()->_tracks[0] << std::endl; 
                        //std::clog << "<log> idxFromParent: " << idxFromParent << std::endl; 
                        
                        //std::clog << "<log> leftUnit->_tracks[0]: " << leftUnit->_tracks[0] << std::endl;
                        //std::clog << "<log> leftUnit->_tracks[0]: " << leftUnit->_tracks[0] << std::endl;
                    }
                    else{
                        unsigned cloestLeftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(enter_offset);
                        unsigned idxFromParent = getParentUnit()->_tracks[unit_offset]._data[cloestLeftOffset].getKey(0);
                        getParentUnit()->_tracks[unit_offset]._data[cloestLeftOffset].setKey(0, borrow.getKey(0));
                        //getParentUnit()->_tracks[unit_offset]._data[data_offset-1].setKey(0, selfUnit->_tracks[0].getMinIndex());
                        unsigned deleteOffset = _tracks[unit_offset].getOffsetByIndex(idx);
                        _tracks[unit_offset].deleteData(idx, side); //? side 
                        {
                            unsigned cloestLeftOffset = _tracks[unit_offset].getClosestLeftOffset(deleteOffset);
                            _tracks[unit_offset]._data[cloestLeftOffset].setKey(0, idxFromParent);
                        }
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

                        //unsigned leftMostOffset = getParentUnit()->_tracks[0].getLeftMostOffset();
                        unsigned idxFromParent = getParentUnit()->_tracks[0]._data[enter_offset].getKey(0);
                        rightUnit->_tracks[unit_offset].insertData(idxFromParent, selfUnit->getSideUnit(), false);
                    }
                    else{
                        getParentUnit()->_tracks[unit_offset]._data[enter_offset].setKey(0, borrow.getKey(0));
                        _tracks[unit_offset].deleteData(idx);
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }
                }

                adjInternalIndex();
                if(leftUnit != nullptr)
                    leftUnit->adjInternalIndex();
                if(rightUnit != nullptr)
                    rightUnit->adjInternalIndex();

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
        
        if(right.isLeaf() && !right.isHalf(0)){
            KeyPtrSet borrow = right._tracks[0].getMinData();
            right._tracks[0].deleteData(right._tracks[0].getMinIndex());
            left._tracks[0].insertData(borrow.getKey(0), borrow.getPtr());
            //std::clog << "<log> destination: " << destination << std::endl;

            return borrow;
        }
        else if(!right.isLeaf() && !right.isHalf(0)){
            KeyPtrSet borrow = right._tracks[0].getMinData();
            right._tracks[0].deleteData(right._tracks[0].getMinIndex());
            ((Unit *)borrow.getPtr())->connectParentUnit(&left);
            left._tracks[0].insertData(borrow.getKey(0), borrow.getPtr());

            return borrow;
        }

        return KeyPtrSet();
    }

    KeyPtrSet borrowDataFromLeft(Unit &left, Unit& right){
        // unit_offset = getRightMostUnit
        if(left.isLeaf() && !left.isHalf(0)){
            KeyPtrSet borrow = left._tracks[0].getMaxData();
            left._tracks[0].deleteData(left._tracks[0].getMaxIndex());
            right._tracks[0].insertData(borrow.getKey(0), borrow.getPtr());

            return borrow;
        }
        else if(!left.isLeaf() && !left.isHalf(0)){
            KeyPtrSet borrow = left._tracks[0].getMaxData();
            borrow.setPtr(left.getSideUnit());
            ((Unit *)borrow.getPtr())->connectParentUnit(&right);
            left._tracks[0].deleteData(left._tracks[0].getMaxIndex(), true);
            right._tracks[0].insertData(borrow.getKey(0), borrow.getPtr(), false);

            return borrow;
        }

        return KeyPtrSet();
    }

    // Merge right Node to left Node
    void mergeNodeFromRight(Unit &left, Unit &right){
        // left_unit_offset = 
        // right_unit_offset = 

        std::clog << "<log> <mergeNodeFromRight()> begin left: " << left._tracks[0] << std::endl;
        std::clog << "<log> <mergeNodeFromRight()> begin right: " << right._tracks[0] << std::endl;

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

        if(left.isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                if(right._tracks[0]._bitmap[i]){
                    left._tracks[0].insertData(right._tracks[0]._data[i].getKey(0), right._tracks[0]._data[i].getPtr());
                    right._tracks[0].deleteMark(i);
                }
            }
        }
        else{
            Unit *tempSide = left.getSideUnit();
            left.connectSideUnit(nullptr);
            bool first = true;
            for(int i = 0; i < _options.track_length; ++i){
                if(right._tracks[0]._bitmap[i]){
                    ((Unit *)right._tracks[0]._data[i].getPtr())->connectParentUnit(&left);
                    left._tracks[0].insertData(right._tracks[0]._data[i].getKey(0), right._tracks[0]._data[i].getPtr(), false);
                    right._tracks[0].deleteMark(i);
                }
            }
            right.getSideUnit()->connectParentUnit(&left);
            left.connectSideUnit(tempSide);
        }

        std::clog << "<log> <mergeNodeFromRight()> end left: " << left._tracks[0] << std::endl;
        std::clog << "<log> <mergeNodeFromRight()> end right: " << right._tracks[0] << std::endl;
    }

    void mergeNodeFromLeft(Unit &left, Unit &right){
        //std::clog << "<log> <mergeNodeFromLeft()> begin left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromLeft()> begin right: " << right._tracks[0] << std::endl;
        
        if(left.isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                if(left._tracks[0]._bitmap[i]){
                    right._tracks[0].insertData(left._tracks[0]._data[i].getKey(0), left._tracks[0]._data[i].getPtr());
                    left._tracks[0].deleteMark(i);
                }
            }
            //std::clog << "<log> left: " << left << std::endl;
            //std::clog << "<log> right: " << right << std::endl;
        }
        else{
            //left.connectSideUnit(right._side);
            for(int i = 0; i < _options.track_length; ++i){
                if(left._tracks[0]._bitmap[i]){
                    ((Unit *)left._tracks[0]._data[i].getPtr())->connectParentUnit(&right);
                    right._tracks[0].insertData(left._tracks[0]._data[i].getKey(0), left._tracks[0]._data[i].getPtr(), false);
                    left._tracks[0].deleteMark(i);
                }
            }
            left.getSideUnit()->connectParentUnit(&right);
            //right._tracks[0].insertData(left.getSideUnit()->_tracks[0].getMinIndex(), left.getSideUnit(), false);
            //std::clog << "<log> left: " << left << std::endl;
            //std::clog << "<log> right: " << right << std::endl;
        }

        //std::clog << "<log> <mergeNodeFromLeft()> end left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromLeft()> end right: " << right._tracks[0] << std::endl;
    }

    Unit *findRightUnit(unsigned unit_offset, unsigned data_offset, unsigned enter_offset, bool &side){
        std::clog << "<log> enter_offset: " << enter_offset << std::endl;
        std::clog << "<log> data_offset: " << data_offset << std::endl;
        //!
        if(_isRoot)
            return nullptr;

        if(enter_offset == _options.track_length){
            //side = true;
            return getParentUnit()->getSideUnit();
        }

        if(getParentUnit()->_tracks[unit_offset].isRightMostOffset(0)){
            side = true;
            return getParentUnit()->getSideUnit();
        }

        if(data_offset == _options.track_length){
            side = true;
            //return getParentUnit()->getSideUnit();
        }
        else if(!getParentUnit()->_tracks[unit_offset].isRightMostOffset(enter_offset)){
            unsigned rightOffset = getParentUnit()->_tracks[unit_offset].getClosestRightOffset(enter_offset);
            return (Unit *)getParentUnit()->_tracks[unit_offset]._data[rightOffset].getPtr();
            //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
        }

        

        return nullptr;
    }

    Unit *findLeftUnit(unsigned unit_offset, unsigned data_offset, unsigned enter_offset){
        if(_isRoot)
            return nullptr;
        
        if(!getParentUnit()->_tracks[unit_offset].isLeftMostOffset(enter_offset)){
            unsigned leftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(enter_offset);
            return (Unit *)getParentUnit()->_tracks[unit_offset]._data[leftOffset].getPtr();
            //std::clog << "<log> leftUnit: " << leftUnit << std::endl;
        }

        return nullptr;
    }

    void adjInternalIndex(){
        Unit *nextUnit = getSideUnit();
        int i = 0;
        if(nextUnit->isLeaf()){
            for(int k = 0; k < _options.track_length; ++k){
                if(_tracks[i]._bitmap[k]){
                    if(_tracks[i].isRightMostOffset(k) || k == _options.track_length-1){
                        _tracks[i]._data[k].setKey(0, getSideUnit()->_tracks[0].getMinIndex());
                    }
                    else if(_tracks[i].getRightMostOffset() != -1){
                        unsigned cloestRightOffset = _tracks[i].getClosestRightOffset(k);
                        Unit *next = (Unit *)_tracks[i]._data[cloestRightOffset].getPtr();
                        _tracks[i]._data[k].setKey(0, next->_tracks[0].getMinIndex());
                    }
                }
            }
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

    // For root unit to check empty
    bool isEmpty(unsigned offset){
        for(int i = 0; i < _options.track_length; ++i){
            if(_tracks[offset]._bitmap[i]){
                return false;
            }
        }
        return true;
    }

    bool isHalf(unsigned offset) const{
        return _tracks[offset].isHalf();
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

    Unit *getRoot(){
        Unit *unit = this;
        while(unit->_tracks[0]._parent != nullptr){
            unit = unit->_tracks[0]._parent;
        }

        return unit;
    }

    Node *_tracks;
    //
    //
    //
    void setRoot(bool status){
        _isRoot = status;
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