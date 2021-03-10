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
        //std::clog << "<log> <deleteData()> idx: " << idx << std::endl;

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
        }
        else{
            for(int i = 0; i < _options.unit_size; ++i){
                for(int j = 0; j < _options.track_length; ++j){
                    Unit *nextUnit = (Unit *)_tracks[i]._data[j].getPtr();
                    if(_tracks[i]._bitmap[j] && nextUnit != nullptr && idx < _tracks[i]._data[j].getKey(0)){
                        nextUnit->deleteData(idx, unit_offset, j, mergeFlag);
                        if(mergeFlag){
                            unsigned deleteIndex = _tracks[i]._data[j].getKey(0);
                            deleteCurrentData(deleteIndex, unit_offset, data_offset, data_offset, mergeFlag);
                            
                        }
                        return;
                    }
                }
                // check side unit
                Unit *nextUnit = getSideUnit();
                nextUnit->deleteData(idx, unit_offset, _options.track_length, mergeFlag);
                if(mergeFlag){
                    //std::clog << "<log> <deleteData()> Unit _id: " << _id << std::endl;
                    unsigned deleteOffset = _tracks[unit_offset].getRightMostOffset();
                    unsigned deleteIndex = _tracks[unit_offset]._data[deleteOffset].getKey(0);
                    //std::clog << "<log> Side deleteOffset: " << deleteOffset << std::endl;
                    //std::clog << "<log> Side deleteIndex: " << deleteIndex << std::endl;
                    deleteCurrentData(deleteIndex, unit_offset, deleteOffset, data_offset, mergeFlag);
                }
                return;
            }

            throw "Delete Fail";
        }
    }

    void deleteCurrentData(unsigned idx, unsigned unit_offset, unsigned data_offset, unsigned enter_offset, bool &mergeFlag){
        std::clog << "<log> <deleteCurrentData()> idx: " << idx << std::endl;

        if(_isRoot){
            _tracks[unit_offset].deleteData(idx);
            if(isEmpty(0)){
                this->setRoot(false);
                getSideUnit()->setRoot(true);
                getSideUnit()->connectParentUnit(nullptr);
            }
        }
        else if(isLeaf()){
            Unit *selfUnit = this;
            Unit *rightUnit = nullptr;
            Unit *leftUnit = nullptr;
            bool side = false;

            std::clog << "<log> Leaf data_offset: " << data_offset << std::endl;
            if(!getParentUnit()->_tracks[unit_offset].isLeftMostOffset(data_offset)){
                unsigned leftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(data_offset);
                leftUnit = (Unit *)getParentUnit()->_tracks[unit_offset]._data[leftOffset].getPtr();
                //std::clog << "<log> leftUnit: " << leftUnit << std::endl;
            }

            if(data_offset == _options.track_length){
                //side = true;
                //rightUnit = getParentUnit()->getSideUnit();
                //std::clog << "<log> rightUnit Side: " << rightUnit << std::endl;
            }
            else if(!getParentUnit()->_tracks[unit_offset].isRightMostOffset(data_offset)){
                unsigned rightOffset = getParentUnit()->_tracks[unit_offset].getClosestRightOffset(data_offset);
                rightUnit = (Unit *)getParentUnit()->_tracks[unit_offset]._data[rightOffset].getPtr();
                //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
            }
            else{
                side = true;
                rightUnit = getParentUnit()->getSideUnit();
            }

            std::clog << "<log> Leaf selfUnit: " << selfUnit << std::endl;
            std::clog << "<log> Leaf leftUnit: " << leftUnit << std::endl;
            std::clog << "<log> Leaf rightUnit: " << rightUnit << std::endl;

            // Check merge or not
            if(isHalf(unit_offset)){
                if(rightUnit != nullptr){
                    KeyPtrSet borrow = borrowDataFromRight(selfUnit->_tracks[unit_offset], rightUnit->_tracks[0]);
                    //std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        mergeNode(selfUnit->_tracks[unit_offset], rightUnit->_tracks[0]);

                        if(side){
                            getParentUnit()->connectSideUnit(selfUnit);
                        }
                        else{
                            unsigned cloestRightOffset = getParentUnit()->_tracks[unit_offset].getClosestRightOffset(data_offset);
                            getParentUnit()->_tracks[unit_offset]._data[cloestRightOffset].setPtr(selfUnit);
                        }
                        mergeFlag = true;
                        //getParentUnit()->deleteCurrentData(deleteIndex, unit_offset, data_offset);
                        
                    }
                    else{
                        getParentUnit()->_tracks[unit_offset]._data[data_offset].setKey(0, rightUnit->_tracks[0].getMinIndex());
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }
                    _tracks[unit_offset].deleteData(idx);
                    if(data_offset >= 1 && getParentUnit()->_tracks[unit_offset]._data[data_offset-1].getKey(0) == idx){
                        getParentUnit()->_tracks[unit_offset]._data[data_offset-1].setKey(0, _tracks[unit_offset].getMinData().getKey(0));
                    }
                }
                // For side unit
                else if(leftUnit != nullptr){
                    KeyPtrSet borrow = borrowDataFromRight(selfUnit->_tracks[0], leftUnit->_tracks[unit_offset]);
                    //std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        std::clog << "<log> Unit _id: " << _id << std::endl;
                        mergeNode(selfUnit->_tracks[unit_offset], leftUnit->_tracks[0]);
                        mergeFlag = true;
                    }
                    else{
                        getParentUnit()->_tracks[unit_offset]._data[data_offset].setKey(0, selfUnit->_tracks[0].getMinIndex());
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }

                    selfUnit->_tracks[unit_offset].deleteData(idx, side); 
                    //unsigned leftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(data_offset);
                    //if(data_offset >= 1 && getParentUnit()->_tracks[unit_offset]._data[leftOffset].getKey(0) == idx){
                    //    getParentUnit()->_tracks[unit_offset]._data[leftOffset].setKey(0, _tracks[unit_offset].getMinData().getKey(0));
                    //}
                    //std::clog << "<log> leftUnit->_tracks[unit_offset]: " << leftUnit->_tracks[unit_offset] << std::endl;
                    /*
                    unsigned rightMostOffset = getParentUnit()->_tracks[unit_offset].getRightMostOffset();
                    if(data_offset >= 1 && getParentUnit()->_tracks[unit_offset]._data[rightMostOffset].getKey(0) == idx){
                        getParentUnit()->_tracks[unit_offset]._data[rightMostOffset].setKey(0, leftUnit->_tracks[unit_offset].getMinIndex());
                    }*/
                }

                return;
            }

            _tracks[unit_offset].deleteData(idx);
            if(data_offset >= 1 && getParentUnit()->_tracks[unit_offset]._data[data_offset-1].getKey(0) == idx){
                getParentUnit()->_tracks[unit_offset]._data[data_offset-1].setKey(0, _tracks[unit_offset].getMinData().getKey(0));
            }
        } // Delete Leaf End
        // Else delete at Internal unit
        else
        {
            Unit *selfUnit = this;
            Unit *rightUnit = nullptr;
            Unit *leftUnit = nullptr;
            bool side = false;
            bool dataSide = false;
            mergeFlag = false;

            std::clog << "<log> data_offset: " << data_offset << std::endl;
            std::clog << "<log> enter_offset: " << enter_offset << std::endl;

            if(!_tracks[unit_offset].isLeftMostOffset(enter_offset)){
                unsigned leftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(enter_offset);
                leftUnit = (Unit *)getParentUnit()->_tracks[unit_offset]._data[leftOffset].getPtr();
                //std::clog << "<log> leftUnit: " << leftUnit << std::endl;
            }

            if(data_offset == _options.track_length){
                //side = true;
                //rightUnit = getParentUnit()->getSideUnit();
                //std::clog << "<log> rightUnit Side: " << rightUnit << std::endl;
            }
            else if(!getParentUnit()->_tracks[unit_offset].isRightMostOffset(enter_offset)){
                unsigned rightOffset = getParentUnit()->_tracks[unit_offset].getClosestRightOffset(enter_offset);
                rightUnit = (Unit *)getParentUnit()->_tracks[unit_offset]._data[rightOffset].getPtr();
                //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
            }
            else{
                side = true;
                rightUnit = getParentUnit()->getSideUnit();
            }

            
            
            std::clog << "<log> selfUnit: " << selfUnit << std::endl;
            std::clog << "<log> leftUnit: " << leftUnit << std::endl;
            std::clog << "<log> rightUnit: " << rightUnit << std::endl;
            std::clog << "<log> side: " << side << std::endl;
            //std::clog << "<log> dataSide: " << dataSide << std::endl;

            if(isHalf(unit_offset)){
                if(rightUnit != nullptr){
                    KeyPtrSet borrow = borrowDataFromRight(selfUnit->_tracks[unit_offset], rightUnit->_tracks[0]);
                    std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        std::clog << "<log> Unit _id: " << _id << std::endl;
                        //unsigned deleteIndex = rightUnit->_tracks[0].getMinIndex();
                        mergeNode(selfUnit->_tracks[unit_offset], rightUnit->_tracks[0]);
                        if(side){
                            getParentUnit()->connectSideUnit(selfUnit);
                        }
                        else{
                            _tracks[unit_offset]._data[data_offset+1].setPtr(_tracks[unit_offset]._data[data_offset].getPtr());
                            unsigned cloestRightOffset = getParentUnit()->_tracks[unit_offset].getClosestRightOffset(data_offset);
                            getParentUnit()->_tracks[unit_offset]._data[cloestRightOffset].setPtr(selfUnit);
                        }
                        mergeFlag = true;
                        
                    }
                    else{
                        getParentUnit()->_tracks[unit_offset]._data[data_offset].setKey(0, rightUnit->_tracks[0].getMinIndex());
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }

                    selfUnit->_tracks[unit_offset].deleteData(idx); 
                }
                // For side unit merge
                else if(leftUnit != nullptr){
                    KeyPtrSet borrow = borrowDataFromRight(selfUnit->_tracks[0], leftUnit->_tracks[unit_offset]);
                    //std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        std::clog << "<log> Unit _id: " << _id << std::endl;
                        //unsigned deleteIndex = selfUnit->_tracks[0].getMinIndex();
                        mergeNode(selfUnit->_tracks[unit_offset], leftUnit->_tracks[0]);
                        mergeFlag = true;
                    }
                    else{
                        getParentUnit()->_tracks[unit_offset]._data[data_offset].setKey(0, selfUnit->_tracks[0].getMinIndex());
                        //_tracks[unit_offset].insertData(rightUnit->_tracks[0].getMinIndex(), rightUnit->_tracks[0].getMinData().getPtr());
                    }

                    data_offset = _tracks[unit_offset].getOffsetByIndex(idx);
                    if(_tracks[unit_offset].isRightMostOffset(data_offset)){
                        dataSide = true;
                    }

                    selfUnit->_tracks[unit_offset].deleteData(idx, dataSide); 
                }

                return;
            }

            //if(_tracks[unit_offset].isRightMostOffset(data_offset)){
            //    dataSide = true;
            //}
            _tracks[unit_offset].deleteData(idx, dataSide);    
        }
    }

    KeyPtrSet borrowDataFromRight(Node &left, Node& right){
        
        if(left._isLeaf && !right.isHalf()){
            KeyPtrSet borrow = right.getMinData();
            right.deleteData(right.getMinIndex());
            left.insertData(borrow.getKey(0), borrow.getPtr());
            //std::clog << "<log> destination: " << destination << std::endl;

            return borrow;
        }
        else if(!left._isLeaf && !right.isHalf()){
            KeyPtrSet borrow = right.getMinData();
            right.deleteData(right.getMinIndex());
            ((Unit *)borrow.getPtr())->connectParentUnit(this);
            left.insertData(borrow.getKey(0), borrow.getPtr());

            return borrow;
        }

        return KeyPtrSet();
    }

    KeyPtrSet borrowDataFromLeft(Node &left, Node& right){
        
        if(left._isLeaf && !left.isHalf()){
            KeyPtrSet borrow = left.getMaxData();
            left.deleteData(left.getMaxIndex());
            right.insertData(borrow.getKey(0), borrow.getPtr());
            //std::clog << "<log> destination: " << destination << std::endl;

            return borrow;
        }
        else if(!left._isLeaf && !left.isHalf()){
            KeyPtrSet borrow = left.getMaxData();
            left.deleteData(left.getMinIndex());
            ((Unit *)borrow.getPtr())->connectParentUnit(this);
            right.insertData(borrow.getKey(0), borrow.getPtr());
        }

        return KeyPtrSet();
    }

    // Merge right Node to left Node
    void mergeNode(Node &left, Node &right){
        std::clog << "<log> <mergeNode()> begin" << std::endl;
        std::clog << "<log> Merge " << right._id << " to " << left._id << std::endl;

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

        copyHalfNode(left, right);
        std::clog << "<log> <mergeNode()> end" << std::endl;
    }

    void copyHalfNode(Node &left, Node &right){
        if(left._isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(right._bitmap[i]){
                    left.insertData(right._data[i].getKey(0), right._data[i].getPtr());
                    right.deleteMark(i);
                }
            }
            std::clog << "<log> left: " << left << std::endl;
            std::clog << "<log> right: " << right << std::endl;
        }
        else{
            left.connectSideUnit(right._side);
            for(int i = 0; i < _options.track_length; ++i){
                if(right._bitmap[i]){
                    ((Unit *)right._data[i].getPtr())->connectParentUnit(this);
                    left.insertData(((Unit *)right._data[i].getPtr())->_tracks[0].getMinIndex(), right._data[i].getPtr());
                    right.deleteMark(i);
                }
            }
            std::clog << "<log> left: " << left << std::endl;
            std::clog << "<log> right: " << right << std::endl;
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