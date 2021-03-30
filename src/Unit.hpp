#ifndef UNIT_H
#define UNIT_H

#include "Options.hpp"
#include "KeyPtrSet.hpp"
#include "Node.hpp"
#include "Counter.hpp"
#include <ostream>

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

    KeyPtrSet *readData(unsigned unit_offset){
        return _tracks[unit_offset].readData(0, _options.track_length);
    }

    /**
     * * Return data pointer
     * * DONE
     */
    unsigned *searchData(unsigned idx, unsigned &unit_offset){
        unsigned *dataPtr = nullptr;

        if(isLeaf()){
            try{
                //std::clog << "<log> <searchData()> idx: " << idx << std::endl;
                //std::clog << "<log> <searchData()> _id: " << _id << std::endl;
                //std::clog << "<log> <searchData()> unit_offset: " << unit_offset << std::endl;
                dataPtr = (unsigned *)_tracks[unit_offset].searchData(idx, unit_offset);
                if(dataPtr == nullptr){
                    if(unit_offset + 1 < _options.unit_size){
                        unit_offset += 1;
                        dataPtr = (unsigned *)_tracks[unit_offset].searchData(idx, unit_offset);
                    }
                }
                if(dataPtr == nullptr){
                    if(hasBackSideUnit()){
                        dataPtr = (unsigned *)getBackSideUnit()->searchData(idx, unit_offset);
                    }
                }
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
    void insertData(unsigned idx, unsigned data, unsigned unit_offset, unsigned data_enter_offset){
        //std::clog << "<log> <insertData()> idx: " << idx << std::endl;

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
            insertCurrentData(idx, data, unit_offset, data_enter_offset);
        }
        else{
            
            
            if(_tracks[unit_offset]._sideBack == nullptr){
                throw "insert side error";
            }
            
            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
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
                            if(_tracks[unit_offset].isLeftMostOffset(i) && unit_offset == 0 && j == 0){
                                ((Unit *)_tracks[unit_offset]._data[i].getPtr())->insertData(idx, data, j+1, i);
                            }
                            else{
                                ((Unit *)_tracks[unit_offset]._data[i].getPtr())->insertData(idx, data, j, i);
                            }
                            
                            return;
                        }
                    }
                }

                KeyPtrSet rightMostData = _tracks[unit_offset].getMaxData();
                unsigned leftMostOffset = _tracks[unit_offset].getLeftMostOffset();

                if(getDataSize(unit_offset) == 1){
                    if(_tracks[unit_offset]._data[leftMostOffset].getPtr() == getBackSideUnit(unit_offset)){
                        ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, _options.unit_size - 1, _options.track_length);
                    }
                    else{
                        ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, 0, _options.track_length);
                    }
                }
                else{
                    if(rightMostData.getPtr() == getBackSideUnit(unit_offset)){
                        ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, _options.unit_size - 1, _options.track_length);
                    }
                    else{
                        ((Unit *)_tracks[unit_offset]._sideBack)->insertData(idx, data, 0, _options.track_length);
                    }
                } 
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
    void insertCurrentData(unsigned idx, unsigned data, unsigned unit_offset, unsigned data_enter_offset){
        std::clog << "<log> <insertCurrentData()> idx: " << idx << std::endl;

        if(!isLeaf())
            throw "This function is for Leaf node";

        if(data_enter_offset == _options.track_length && !isRoot()){
            data_enter_offset = getParentUnit()->_tracks[getParentOffset()].getRightMostOffset();
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
            //* if unit is not full, do balance
            if(isFull(unit_offset) && !isFullUnit()){
                //* if all valid, just need to balance
                if(isAllValid()){
                    Node *leftNode = nullptr, *rightNode = nullptr;

                    if(unit_offset < (unit_offset+1) % 2){
                        leftNode = &_tracks[unit_offset];
                        rightNode = &_tracks[(unit_offset+1) % 2];
                        std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        balanceDataFromLeft(*leftNode, *rightNode);
                        std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[0]._data[enter_offset]: " << getParentUnit()->_tracks[0] << std::endl;
                        if(!isRoot()){
                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].setKey(0, rightNode->getMinIndex());
                            //getParentUnit()->adjInternalIndex(0);
                        }
                        
                        std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }
                    else{
                        leftNode = &_tracks[(unit_offset+1) % 2];
                        rightNode = &_tracks[unit_offset];
                        balanceDataFromRight(*leftNode, *rightNode);

                        if(!isRoot()){
                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].setKey(0, rightNode->getMinIndex());
                            //getParentUnit()->adjInternalIndex(0);
                            //std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[0]._data[enter_offset]: " << getParentUnit()->_tracks[0] << std::endl;
                            //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                            //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                        }
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }

                    getRoot()->insertData(idx, data, 0, -1);
                    
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

                //std::clog << "<log> this->_tracks[0]._parent: " << this->_tracks[0]._parent << std::endl;
                getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                // insert new index from root
                //? or from its parent
                getRoot()->insertData(idx, data, 0, -1);

                return;
            }
        }

        //* direct insert index and data as data pointer to node
        _tracks[unit_offset].insertData(idx, new unsigned(data));
    }

    void insertCurrentPointer(unsigned idx, Unit *unit, unsigned unit_offset, unsigned data_enter_offset){
        std::clog << "<log> <insertCurrentPointer()> idx: " << idx << std::endl;
        std::clog << "<log> <insertCurrentPointer()> unit: " << unit << std::endl;
        std::clog << "<log> <insertCurrentPointer()> offset: " << unit_offset << std::endl;

        if(isLeaf())
            throw "This function is for Internal node";
        
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
                        insertCurrentPointer(idx, unit, unit_offset, 0);
                        unit->connectParentUnit(this, 0);
                    }
                    else if(idx > promote.getKey(0)){
                        rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(rightUnit, 0);
                    }
                    else{ 
                        unsigned leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                        rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                        unit->connectParentUnit(rightUnit, 0);
                    }

                    return;
                }
                this->_tracks[0]._parent->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0, 0);
                
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
                    unsigned leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                    rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                    unit->connectParentUnit(rightUnit, 0);
                }

                return;
            }
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            if(isFull(unit_offset)){
                KeyPtrSet promote = splitNode(idx, unit_offset);

                std::clog << "<log> idx: " << idx << std::endl;
                std::clog << "<log> promote.getKey(0): " << promote.getKey(0) << std::endl;

                if(promote.getKey(0) == 0)
                    return;

                if(_isRoot){
                    setRoot(false);

                    Unit *newRoot = System::allocUnit(_options, false);
                    newRoot->setLeaf(false);

                    newRoot->insertCurrentPointer(*promote.key, this, 0, 0);
                    this->connectParentUnit(newRoot, 0);

                    newRoot->connectBackSideUnit((Unit *)promote.ptr, 0);
                    ((Unit *)promote.ptr)->connectParentUnit(newRoot, 0);
                    
                    Unit *rightUnit = (Unit *)promote.ptr;

                    if(idx < promote.getKey(0)){
                        insertCurrentPointer(idx, unit, unit_offset, 0);
                        unit->connectParentUnit(this, 0);
                    }
                    else if(idx > promote.getKey(0)){
                        rightUnit->insertCurrentPointer(idx, unit, _options.unit_size - 1, 0);
                        unit->connectParentUnit(rightUnit, 1);
                    }
                    else{ 
                        unsigned leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                        rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                        unit->connectParentUnit(rightUnit, 1);
                    }
                    //std::clog << "<log> newRoot: " << (*newRoot)._tracks[0] << std::endl;
                    return;
                }
                //insertCurrentPointer(*promote.key, (Unit *)promote.ptr, 0);
                //((Unit *)promote.ptr)->connectParentUnit(this, 0);
                
                //! getParentOffset()
                this->_tracks[0]._parent->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);
                
                ////if(((Unit *)promote.ptr)->getParentUnit() == nullptr)
                    ////((Unit *)promote.ptr)->connectParentUnit(getParentUnit());
                Unit *rightUnit = (Unit *)promote.ptr;
                if(idx < promote.getKey(0)){
                    insertCurrentPointer(idx, unit, 0, 0);
                    unit->connectParentUnit(this, 1);
                }
                else if(idx > promote.getKey(0)){
                    rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                    unit->connectParentUnit(rightUnit, 0);
                }
                else{
                    unsigned leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                    rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                    //((Unit *)rightUnit->_tracks[0]._data[0].getPtr())->connectParentUnit(rightUnit);
                    unit->connectParentUnit(rightUnit, 0);
                    //std::clog << "<log> rightUnit->_tracks[0]._data[0].getPtr(): " << rightUnit->_tracks[0]._data[0].getPtr() << std::endl;
                    //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
                }

                return;
            }
        }

        //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[offset] << std::endl;  
        _tracks[unit_offset].insertData(idx, unit);
        unit->connectParentUnit(this, unit_offset);
    }

    void connectBackSideUnit(Unit *unit, unsigned unit_offset = 0){
        _tracks[unit_offset].connectBackSideUnit(unit);
    }

    void connectFrontSideUnit(Unit *unit, unsigned unit_offset = 0){
        _tracks[unit_offset].connectFrontSideUnit(unit);
    }

    bool hasBackSideUnit(unsigned unit_offset = 0) const{
        return _tracks[unit_offset]._sideBackBitmap;
    }

    bool hasFrontSideUnit(unsigned unit_offset = 0) const{
        return _tracks[unit_offset]._sideFrontBitmap;
    }

    Unit *getBackSideUnit(unsigned unit_offset = 0) const{
        return _tracks[unit_offset]._sideBack;
    }

    Unit *getFrontSideUnit(unsigned unit_offset = 0) const{
        return _tracks[unit_offset]._sideFront;
    }

    /**
     * * DONE
    */
    void connectParentUnit(Unit *unit, unsigned offset = 0){
        for(int i = 0; i < _options.unit_size; ++i)
            _tracks[i].connectParentNode(unit, offset);
    }

    Unit *getParentUnit() const{
        return _tracks[0]._parent;
    }

    unsigned getParentOffset() const{
        return _tracks[0]._parentOffset;
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

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            // split operation will generate same level unit/node
            Unit *newUnit = System::allocUnit(_options, isLeaf());

            // it is never root
            newUnit->setRoot(false);

            // find the middle index
            unsigned promoteKey = System::getMid(_tracks[0]._data, _options.track_length, wait_insert_idx);
            
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
            bool side = false;
            if(!isRoot()){
                if(getParentUnit()->getDataSize(0) != 1 && getParentUnit()->getBackSideUnit() == this){
                    side = true;
                }
                //std::clog << "<log> getParentUnit()->getSize(0): " << getParentUnit()->getSize(0) << std::endl;
            }

            //* In-Unit Split
            if(!isAllValid()){
                // find the middle index
                unsigned promoteKey = System::getMid(_tracks[0]._data, _options.track_length, wait_insert_idx);

                // combine as a key-point set
                promote.setPtr(this);
                promote.addKey(promoteKey);

                balanceDataFromLeft(_tracks[0], _tracks[1], promote);
            }
            //* Unit-Unit Split
            else if(isFullUnit()){
                //std::clog << "<log> getParentUnit()->getValidSize(): " << getParentUnit()->getValidSize() << std::endl;
                if(hasBackSideUnit() && !getBackSideUnit()->isAllValid()){
                    getBackSideUnit()->migrateNode(getBackSideUnit()->_tracks[0], getBackSideUnit()->_tracks[1]);

                    unsigned promoteKey = System::getMid(_tracks[1]._data, _options.track_length, wait_insert_idx);
                
                    promote.setPtr(getBackSideUnit());
                    promote.addKey(promoteKey);

                    // copying data
                    copyHalfNode(_tracks[1], getBackSideUnit()->_tracks[0], promote, wait_insert_idx);
                }
                else{
                    // split operation will generate same level unit/node
                    Unit *newUnit = System::allocUnit(_options, isLeaf());

                    // find the middle index
                    unsigned promoteKey = System::getMid(_tracks[1]._data, _options.track_length, wait_insert_idx);

                    // combine as a key-point set
                    promote.setPtr(newUnit);
                    promote.addKey(promoteKey);

                    // it is never root
                    newUnit->setRoot(false);

                    // split operation will generate same level unit/node
                    if(!isLeaf()) newUnit->setLeaf(false);

                    //* copying data
                    copyHalfNode(_tracks[1], newUnit->_tracks[0], promote, wait_insert_idx);
                    //std::clog << "<log> newUnit->_tracks[0]: " << newUnit->_tracks[0] << std::endl;

                    //* sure to have correct parent pointers
                    if(!isLeaf()){
                        getBackSideUnit()->connectParentUnit(this);
                        newUnit->getBackSideUnit()->connectParentUnit(newUnit);
                    }
                }

            }
            else{
                throw "split error";
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
    void copyHalfNode(Node &source, Node &destination, KeyPtrSet promote, unsigned wait_insert_idx){
        if(isLeaf()){
            switch(_options.node_ordering){
                case Options::ordering::SORTED:
                    if(wait_insert_idx < promote.getKey(0)){
                        for(int i = _options.track_length / 2 - 1, j = 0; i < _options.track_length; ++i, ++j){
                            destination._data[j] = source._data[i];

                            source._data[i].delAll();
                        }
                    }
                    else{
                        for(int i = _options.track_length / 2, j = 0; i < _options.track_length; ++i, ++j){
                            destination._data[j] = source._data[i];

                            source._data[i].delAll();
                        }
                    }
                    //std::clog << "<log> destination: " << destination << std::endl;
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
            destination.connectBackSideUnit(source._sideBack);

            std::clog << "<log> <copyHalfNode()> source: " << source << std::endl;
            std::clog << "<log> <copyHalfNode()> destination: " << destination << std::endl;
            
            bool promoteMid = true;
            for(int i = 0; i < _options.track_length; ++i){
                if(source._data[i].getBitmap(0) && source._data[i].getKey(0) == promote.getKey(0)){
                    std::clog << "<log> <copyHalfNode()> source" << std::endl;
                    //source.deleteMark(i);
                    source.deleteData(promote.getKey(0));
                    promoteMid = false;
                    source.connectBackSideUnit((Unit *)source._data[i].getPtr());
                }
            }
            for(int i = 0; i < _options.track_length; ++i){
                if(destination._data[i].getBitmap(0) && destination._data[i].getKey(0) == promote.getKey(0)){
                    std::clog << "<log> <copyHalfNode()> destination" << std::endl;
                    //destination.deleteMark(i);
                    destination.deleteData(promote.getKey(0));
                    promoteMid = false;
                    source.connectBackSideUnit((Unit *)destination._data[0].getPtr());
                }
            }

            if(promoteMid){
                source.connectBackSideUnit((Unit *)destination._data[0].getPtr());
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
                Unit *nextUnit = getBackSideUnit();
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

                        unsigned cloestLeftOffset = getParentUnit()->_tracks[0].getClosestLeftOffset(enter_offset);
                        unsigned idxFromParent = getParentUnit()->_tracks[0]._data[cloestLeftOffset].getKey(0);
                        //
                        selfUnit->_tracks[unit_offset].deleteData(idx, side);
                        leftUnit->_tracks[unit_offset].insertData(idxFromParent, leftUnit->getBackSideUnit(), false);
                        
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
                        rightUnit->_tracks[unit_offset].insertData(idxFromParent, selfUnit->getBackSideUnit(), false);
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
            borrow.setPtr(left.getBackSideUnit());
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
            Unit *tempSide = left.getBackSideUnit();
            left.connectBackSideUnit(nullptr);
            bool first = true;
            for(int i = 0; i < _options.track_length; ++i){
                if(right._tracks[0]._data[i].getBitmap(0)){ //TODO
                    ((Unit *)right._tracks[0]._data[i].getPtr())->connectParentUnit(&left);
                    left._tracks[0].insertData(right._tracks[0]._data[i].getKey(0), right._tracks[0]._data[i].getPtr(), false);
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
        if(isLeaf()){
            if(right.getSize() - left.getSize() == 1){
                left.insertData(right._data[0].getKey(0), right._data[0].getPtr());
                right.deleteData(right._data[0].getKey(0));
            }
            else{
                for(int i = 0; i < (right.getSize() - left.getSize()) / 2 ; ++i){
                    left.insertData(right._data[i].getKey(0), right._data[i].getPtr());
                    right.deleteData(right._data[i].getKey(0));
                }
            }
            
        }
        else{

        }

        //std::clog << "<log> <balanceDataFromRight()> left: " << left << std::endl;
        //std::clog << "<log> <balanceDataFromRight()> right: " << right << std::endl;
    }

    /**
     * * for double tracks
     * @param left is full node
     * @param right is non-full node
    */
    void balanceDataFromLeft(Node &left, Node &right, KeyPtrSet promote = KeyPtrSet()){
        unsigned begin = (left.getSize() + right.getSize()) / 2;

        if(isLeaf()){
            for(int i = begin; i < _options.track_length; ++i){
                right.insertData(left._data[i].getKey(0), left._data[i].getPtr());
                left.deleteData(left._data[i].getKey(0));
            }
            right.setValid(true);
        }
        else{
            left._sideBack->connectParentUnit(this, 1);
            right.connectBackSideUnit(left._sideBack);

            left.deleteData(promote.getKey(0));
            for(int i = begin; i < _options.track_length; ++i){
                ((Unit *)left._data[i].getPtr())->connectParentUnit(this, 1);
                right.insertDataFromBack(left._data[i]);
                left.deleteMark(i);
            }
            unsigned rightMostOffset = left.getRightMostOffset();
            left.connectBackSideUnit((Unit *)left._data[rightMostOffset].getPtr());

            right.setValid(true);
        }
        //std::clog << "<log> <balanceDataFromLeft()> left: " << left << std::endl;
        //std::clog << "<log> <balanceDataFromLeft()> right: " << right << std::endl;
    }

    /**
     * * for double tracks
     * * From left to right
     * @param left
     * @param right
    */
    void migrateNode(Node &left, Node &right){
        if(isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                if(left._data[i].getBitmap(0)){
                    right.insertData(left._data[i].getKey(0), left._data[i].getPtr());
                    left.deleteData(left._data[i].getKey(0));
                }
            }
            right.setValid(true);
        }
        else{

        }
        std::clog << "<log> <migrateNode()> left: " << left << std::endl;
        std::clog << "<log> <migrateNode()> right: " << right << std::endl;
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
            return getParentUnit()->getBackSideUnit();
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
        std::clog << "<log> <adjInternalIndex()> begin" << std::endl;
        if(!isLeaf()){
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(_tracks[unit_offset]._data[i].getBitmap(j)){ //TODO
                        ((Unit *)_tracks[unit_offset]._data[i].getPtr())->adjInternalIndex(j);

                        Unit *nextUnit = nullptr;
                        std::clog << "<log> <adjInternalIndex()> nextUnit: " << nextUnit << std::endl;
                        if(_tracks[unit_offset].isRightMostOffset(i)){
                            nextUnit = getBackSideUnit(unit_offset);
                        }
                        else{
                            unsigned cloestRightOffset = _tracks[unit_offset].getClosestRightOffset(i);
                            nextUnit = (Unit *)_tracks[unit_offset]._data[cloestRightOffset / _options.unit_size].getPtr();
                        }
                        //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0]: " << nextUnit->_tracks[0] << std::endl;
                        std::clog << "<log> <adjInternalIndex()> nextUnit: " << nextUnit << std::endl;

                        if(nextUnit->isLeaf()){
                            _tracks[unit_offset]._data[i].setKey(j, nextUnit->_tracks[j].getMinIndex());
                            //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0].getMinIndex(): " << nextUnit->_tracks[0].getMinIndex() << std::endl;
                        }
                        else{
                            while(!nextUnit->isLeaf()){
                                //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0] ori: " << nextUnit->_tracks[0] << std::endl;
                                unsigned leftMostOffset = nextUnit->_tracks[j].getLeftMostOffset();
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

    bool isFull(unsigned offset) const{
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

    bool isValid(unsigned offset) const{
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

    void setLeaf(bool isLeaf){
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i]._isLeaf = isLeaf;
        }
    }

    unsigned getDataSize(unsigned offset) const{
        return _tracks[offset].getSize();
    }

    unsigned getValidSize() const{
        unsigned counter = 0;
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
    out << "\t\t" << "Parent: " << right.getParentUnit() << " Offset " << right.getParentOffset()  << "\n";
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

            unsigned rightMostOffset = right._tracks[i].getRightMostOffset();
            if(right._tracks[i]._sideBackBitmap && right._tracks[i]._sideBack != nullptr && right._tracks[i]._data[rightMostOffset].getPtr() != right._tracks[i]._sideBack){
                out << *(Unit *)right._tracks[i]._sideBack;
            }
        }
    }
    return out;
}

#endif