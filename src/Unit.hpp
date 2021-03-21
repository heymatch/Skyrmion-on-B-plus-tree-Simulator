#ifndef UNIT_H
#define UNIT_H

#include "Options.hpp"
#include "KeyPtrSet.hpp"
#include "Node.hpp"
#include "Counter.hpp"
#include <ostream>

namespace System{
    Unit *allocUnit(Options options);
}

struct Unit{
    Unit(Options options) : _options(options){
        // init tracks
        _tracks = new Node[_options.unit_size];
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i] = Node(_options, false);
        }

        // assume new unit is root
        _isRoot = true;

        // get unit id
        _id = UnitId++;
    }

    Unit(const Unit &right){
        _tracks = new Node[_options.unit_size];
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i] = right._tracks[i];
        }

        // assume new unit is root
        _isRoot = right._isRoot;

        // get unit id
        _id = right._id;
    }

    /* Major Functions */

    KeyPtrSet *readData(unsigned unit_offset){
        return _tracks[unit_offset].readData(0, _options.track_length);
    }

    /**
     * * Return data pointer
     * * DONE
     */
    unsigned *searchData(unsigned idx, unsigned unit_offset){
        unsigned *dataPtr = nullptr;

        if(isLeaf()){
            try{
                dataPtr = (unsigned *)_tracks[unit_offset].searchData(idx, unit_offset);
            }
            catch(int e){
                if(e != -1) throw "search error";
            }
        }
        else{
            try{
                dataPtr = (unsigned *)((Unit *)_tracks[unit_offset].searchData(idx, unit_offset))->searchData(idx, unit_offset);
            }
            catch(int e){
                if(e != -1) throw "search error";
            }
        }
        
        return dataPtr;
    }

    void updateData(unsigned idx, unsigned data, unsigned offset){

    }

    /**
     * * Controller of insertion
     * TODO evaluation
     */
    void insertData(unsigned idx, unsigned data, unsigned unit_offset){
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
            insertCurrentData(idx, data, unit_offset);
        }
        else{
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(
                        _tracks[unit_offset]._data[i].getBitmap(j) &&
                        _tracks[unit_offset]._data[i].getPtr() != nullptr &&
                        idx < _tracks[unit_offset]._data[i].getKey(j)
                    ){
                        ((Unit *)_tracks[unit_offset]._data[i].getPtr())->insertData(idx, data, j);
                        return;
                    }
                }
            }
            //! Assume side unit only has one track availiable
            if(_tracks[unit_offset]._side != nullptr){
                ((Unit *)_tracks[unit_offset]._side)->insertData(idx, data, 0);
                return;
            }
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
    void insertCurrentData(unsigned idx, unsigned data, unsigned unit_offset){
        if(!isLeaf())
            throw "This function is for Leaf node";

        if(isFull(unit_offset)){
            //* get the middle key and the pointer of new split unit
            KeyPtrSet promote = splitNode(idx, unit_offset);

            // need to grow this tree
            if(_isRoot){
                setRoot(false);

                // allocate a new root unit
                Unit *newRoot = System::allocUnit(_options);
                //* it must be internal unit
                newRoot->setLeaf(false);

                // new root unit insert current unit
                newRoot->insertCurrentPointer(*promote.key, this, 0);
                this->connectParentUnit(newRoot);

                // new root unit insert new split unit
                newRoot->connectSideUnit((Unit *)promote.ptr);
                ((Unit *)promote.ptr)->connectParentUnit(newRoot);

                // side pointer connect to new split unit
                connectSideUnit((Unit *)promote.ptr);

                // insert new index to self unit or new split unit
                if(idx < promote.getKey(unit_offset))
                    insertCurrentData(idx, data, unit_offset);
                else
                    getSideUnit()->insertCurrentData(idx, data, 0);

                return;
            }
            //std::clog << "<log> this->_tracks[0]._parent: " << this->_tracks[0]._parent << std::endl;
            //! Need unit_enter_offset at 3rd parameter
            _tracks[unit_offset]._parent->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
            ////if(((Unit *)promote.ptr)->getParentUnit() == nullptr)
                ////((Unit *)promote.ptr)->connectParentUnit(getParentUnit());

            // insert new index from root
            //? or from its parent
            getRoot()->insertData(idx, data, 0);

            return;
        }

        // insert index and data as data pointer to node
        _tracks[unit_offset].insertData(idx, new unsigned(data));
    }

    void insertCurrentPointer(unsigned idx, Unit *unit, unsigned offset){
        if(isLeaf())
            throw "This function is for Internal node";

        if(isFull(offset)){
            KeyPtrSet promote = splitNode(idx, offset);

            if(_isRoot){
                setRoot(false);

                Unit *newRoot = new Unit(_options);
                newRoot->setLeaf(false);

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
            ////if(((Unit *)promote.ptr)->getParentUnit() == nullptr)
                ////((Unit *)promote.ptr)->connectParentUnit(getParentUnit());
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

    //? no use
    /*
    Unit *getParentRightUnit(unsigned idx) const{
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
    KeyPtrSet splitNode(unsigned wait_insert_idx, unsigned unit_offset){
        KeyPtrSet promote(2);

        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
        {
            Unit *newUnit = System::allocUnit(_options);

            // it is never root
            newUnit->setRoot(false);

            // split operation will generate same level unit/node
            if(!isLeaf()) newUnit->setLeaf(false);

            // find the middle index
            unsigned promoteKey = System::getMid(_tracks[unit_offset]._data, _options.track_length, wait_insert_idx);
            
            // combine as a key-point set
            promote.setPtr(newUnit);
            promote.addKey(promoteKey);
            
            // copying data
            copyHalfNode(_tracks[unit_offset], newUnit->_tracks[0], promote, wait_insert_idx);
            //std::clog << "<log> newUnit->_tracks[0]: " << newUnit->_tracks[0] << std::endl;

            // sure to have correct parent pointers
            if(!isLeaf()){
                getSideUnit()->connectParentUnit(this);
                newUnit->getSideUnit()->connectParentUnit(newUnit);
            }

            break;
        }
        case Options::split_merge_function::UNIT:
            //* In-Unit Split
            //* Unit-Unit Split
            
            throw "Developing split unit";
        default:
            throw "undefined split operation";
        }

        return promote;
    }

    /**
     * * Copy half data from source to destination
     * TODO evaluation
     */
    void copyHalfNode(Node &source, Node &destination, KeyPtrSet promote, unsigned wait_insert_idx){
        if(isLeaf()){
            switch(_options.node_ordering){
                case Options::ordering::SORTED:
                    if(wait_insert_idx < promote.getKey(0)){
                        for(int i = _options.track_length / 2 - 1, j = 0; i < _options.track_length; ++i, ++j){
                            destination._data[j] = source._data[i];

                            //destination._bitmap[j] = true;
                            //source._bitmap[i] = false;
                            source._data[i].delAll();
                        }
                    }
                    else{
                        for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                            destination._data[j] = source._data[i];

                            //destination._bitmap[j] = true;
                            //source._bitmap[i] = false;
                            source._data[i].delAll();
                        }
                    }
                    break;
                case Options::ordering::UNSORTED:
                    throw "unsorted split developing";
                default:
                    throw "undefined ordering";
            }
        }
        else{
            unsigned mid = _options.track_length / 2;
            if(wait_insert_idx < promote.getKey(0)){
                for(int i = _options.track_length / 2 - 1, j = 0; i < _options.track_length; ++i, ++j){
                    destination._data[j] = source._data[i];

                    //destination._bitmap[j] = true;
                    //source._bitmap[i] = false;
                    source._data[i].delAll();

                    ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr());
                }
            }
            else{
                for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                    destination._data[j] = source._data[i];

                    //destination._bitmap[j] = true;
                    //source._bitmap[i] = false;
                    source._data[i].delAll();

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
    
    /**
     * * control data deletion
     * TODO evaluation 
     * TODO general unit parameter
     */
    void deleteData(unsigned idx, unsigned unit_offset, unsigned enter_offset, bool &mergeFlag){
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
                            unsigned deleteIndex = _tracks[i]._data[j].getKey(0);
                            //std::clog << "<log> <deleteData()> deleteIndex: " << deleteIndex << std::endl;
                            //std::clog << "<log> <deleteData()> idx: " << idx << std::endl;
                            deleteCurrentData(deleteIndex, unit_offset, j, enter_offset, mergeFlag);
                        }
                        //adjInternalIndex();
                        return;
                    }
                }
                // check side unit
                Unit *nextUnit = getSideUnit();
                nextUnit->deleteData(idx, unit_offset, _options.track_length, mergeFlag);
                //std::clog << "<log> <deleteData()> side mergeFlag: " << mergeFlag << std::endl;
                //std::clog << "<log> <deleteData()> _tracks[unit_offset]._data[_options.track_length-1].getKey(0): " << _tracks[unit_offset]._data[_options.track_length-1].getKey(0) << std::endl;
                //std::clog << "<log> <deleteData()> idx: " << idx << std::endl;

                if(mergeFlag){
                    //std::clog << "<log> <deleteData()> Unit _id: " << _id << std::endl;
                    unsigned deleteOffset = _tracks[unit_offset].getRightMostOffset();
                    unsigned deleteIndex = _tracks[unit_offset]._data[deleteOffset].getKey(0);
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
    void deleteCurrentData(unsigned idx, unsigned unit_offset, unsigned data_offset, unsigned enter_offset, bool &mergeFlag){
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
                if(getSideUnit() != nullptr){
                    getSideUnit()->setRoot(true);
                    getSideUnit()->connectParentUnit(nullptr);
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
                        //std::clog << "<log> getParentUnit()->_tracks[0]: " << getParentUnit()->_tracks[0] << std::endl; 
                        if(enter_offset == _options.track_length){
                            //getParentUnit()->_tracks[0]._data[cloestLeftOffset].setKey(0, getParentUnit()->_tracks[0]._data[enter_offset].getKey(0));
                            getParentUnit()->_tracks[0].connectSideUnit((Unit *)getParentUnit()->_tracks[0]._data[cloestLeftOffset].getPtr());
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
                        selfUnit->_tracks[unit_offset].deleteData(idx, side); 
                        mergeNodeFromLeft(*selfUnit, *rightUnit);
                        mergeFlag = true;

                        //unsigned leftMostOffset = getParentUnit()->_tracks[0].getLeftMostOffset();
                        unsigned idxFromParent = getParentUnit()->_tracks[0]._data[enter_offset].getKey(0);
                        rightUnit->_tracks[unit_offset].insertData(idxFromParent, selfUnit->getSideUnit(), false);
                    }
                    else{
                        getParentUnit()->_tracks[unit_offset]._data[enter_offset].setKey(0, borrow.getKey(0));
                        _tracks[unit_offset].deleteData(idx, side);
                        _tracks[unit_offset].insertData(borrow.getKey(0), borrow.getPtr());
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
        //! left_unit_offset = 
        //! right_unit_offset = 
        
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
            //left._tracks[0].insertData(borrow.getKey(0), borrow.getPtr());

            return borrow;
        }

        return KeyPtrSet();
    }

    KeyPtrSet borrowDataFromLeft(Unit &left, Unit &right){
        //! left_unit_offset = 
        //! right_unit_offset = 

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
                if(right._tracks[0]._data[i].getBitmap(0)){ //TODO
                    ((Unit *)right._tracks[0]._data[i].getPtr())->connectParentUnit(&left);
                    left._tracks[0].insertData(right._tracks[0]._data[i].getKey(0), right._tracks[0]._data[i].getPtr(), false);
                    right._tracks[0].deleteMark(i);
                }
            }
            right.getSideUnit()->connectParentUnit(&left);
            left.connectSideUnit(tempSide);
        }

        //std::clog << "<log> <mergeNodeFromRight()> end left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromRight()> end right: " << right._tracks[0] << std::endl;
    }

    void mergeNodeFromLeft(Unit &left, Unit &right){
        //! left_unit_offset = 
        //! right_unit_offset = 
        //std::clog << "<log> <mergeNodeFromLeft()> begin left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromLeft()> begin right: " << right._tracks[0] << std::endl;
        
        if(left.isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                if(left._tracks[0]._data[i].getBitmap(0)){ //TODO
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
                if(left._tracks[0]._data[i].getBitmap(0)){ //TODO
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

    /**
     * * for double tracks
     * @param left is full node
     * @param right is non-full node
    */
    void balanceData(Node &left, Node &right){
        if(isLeaf()){
            for(int i = 0; i < _options.track_length / 2; ++i){
                right.insertData(left._data[i].getKey(0), left._data[i].getPtr());
                left.deleteData(left._data[i].getKey(0));
            }
        }
        else{

        }
    }

    /* Minor Functions */

    Unit *findRightUnit(unsigned unit_offset, unsigned data_offset, unsigned enter_offset){
        //std::clog << "<log> enter_offset: " << enter_offset << std::endl;
        //std::clog << "<log> data_offset: " << data_offset << std::endl;
        
        if(_isRoot)
            return nullptr;

        if(enter_offset == _options.track_length){
            return nullptr;
        }

        if(getParentUnit()->_tracks[unit_offset].isRightMostOffset(enter_offset)){
            return getParentUnit()->getSideUnit();
        }

        if(!getParentUnit()->_tracks[unit_offset].isRightMostOffset(enter_offset)){
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

    /**
     * * adjust internal unit index
     * ! most inefficient
     * TODO general unit parameter
     * TODO make it just checking current unit/node
    */
    void adjInternalIndex(unsigned unit_offset = 0){

        if(!isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                if(_tracks[0]._data[i].getBitmap(0)){ //TODO
                    ((Unit *)_tracks[0]._data[i].getPtr())->adjInternalIndex();
                    Unit *nextUnit = nullptr;

                    if(_tracks[0].isRightMostOffset(i)){
                        nextUnit = getSideUnit();
                    }
                    else{
                        unsigned cloestRightOffset = _tracks[0].getClosestRightOffset(i);
                        nextUnit = (Unit *)_tracks[0]._data[cloestRightOffset].getPtr();
                    }
                    //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0]: " << nextUnit->_tracks[0] << std::endl;
                    //std::clog << "<log> <adjInternalIndex()> nextUnit: " << nextUnit << std::endl;

                    if(nextUnit->isLeaf()){
                        _tracks[0]._data[i].setKey(0, nextUnit->_tracks[0].getMinIndex());
                        //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0].getMinIndex(): " << nextUnit->_tracks[0].getMinIndex() << std::endl;
                    }
                    else{
                        
                        while(!nextUnit->isLeaf()){
                            //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0] ori: " << nextUnit->_tracks[0] << std::endl;
                            unsigned leftMostOffset = nextUnit->_tracks[0].getLeftMostOffset();
                            nextUnit = (Unit *)nextUnit->_tracks[0]._data[leftMostOffset].getPtr();
                            //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0] next: " << nextUnit->_tracks[0] << std::endl;
                        }
                        //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0].getMinIndex(): " << nextUnit->_tracks[0].getMinIndex() << std::endl;
                        _tracks[0]._data[i].setKey(0, nextUnit->_tracks[0].getMinIndex());
                    }
                    
                }
            }
            getSideUnit()->adjInternalIndex();
        }
    }

    /* Supported Functions */

    bool isFull(unsigned offset) const{
        for(int i = 0; i < _options.track_length; ++i){
            if(!_tracks[offset]._data[i].getBitmap(0)){ //TODO
                return false;
            }
        }
        return true;
    }

    bool isFullUnit() const{
        for(int i = 0; i < _options.unit_size; ++i){
            if(!isFull(i)){
                return false;
            }
        }

        return true;
    }

    // For root unit to check empty
    bool isEmpty(unsigned offset){
        for(int i = 0; i < _options.track_length; ++i){
            if(_tracks[offset]._data[i].getBitmap(0)){ //TODO
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

    //?
    bool isValid(unsigned offset) const{

    }

    void setLeaf(bool isLeaf){
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i]._isLeaf = isLeaf;
        }
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

    bool _isRoot;
    Options _options;
    unsigned _id;

    Counter _readCounter = Counter("Read");
    Counter _shiftCounter = Counter("Shift");
    Counter _insertCounter = Counter("Insert");
    Counter _removeCounter = Counter("Remove");
    Counter _migrateCounter = Counter("Migrate");
};

#include<unordered_map>
namespace System{
    // store unit pointer
    // store how many the tracks of the unit is using
    std::unordered_map<Unit *, unsigned> unitPool;
    
    Unit *allocUnit(Options options){
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
            newUnit = new Unit(options);
            unitPool[newUnit] = 1;
        }
        
        return newUnit;
    }
}

std::ostream &operator<<(std::ostream &out, const Unit &right){
    //std::clog << "<log> Unit " << right._id << " Print" << std::endl;
    out << "\t[\t\n";
    //* status
    out << "\t\t";
    if(right.isLeaf()) out << "Leaf Unit";
    else out << "Internal Unit";
    out << " " << right._id << " " << &right << "\n";
    //out << " Parent: " << right.getParentUnit();
    out << "\t\t";
    if(right._isRoot) out << "Root";
    out << "\n";

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
            if(right._tracks[i]._sideBitmap && right._tracks[i]._side != nullptr){
                out << *(Unit *)right._tracks[0]._side;
            }
        }
    }
    return out;
}

#endif