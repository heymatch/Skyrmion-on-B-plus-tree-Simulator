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

    void deleteData(unsigned idx, unsigned offset){
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
            deleteCurrentData(idx, offset);
        }
        else{
            for(int i = 0; i < _options.unit_size; ++i){
                for(int j = 0; j < _options.track_length; ++j){
                    
                    Unit *nextUnit = (Unit *)_tracks[i]._data[j].getPtr();

                    if(_tracks[i]._bitmap[j] && nextUnit != nullptr && idx < _tracks[i]._data[j].getKey(0)){
                        nextUnit->deleteData(idx, offset);
                        return;
                    }
                }
                // check side unit
                
                Unit *nextUnit = getSideUnit();
                /*
                unsigned leftOffset = _tracks[i].getClosestLeftIndex(_options.track_length);
                Unit *nextLeftUnit = (Unit *)_tracks[i]._data[leftOffset].getPtr();

                if(nextUnit != nullptr && nextUnit->isLeaf()){
                    if(nextUnit->isHalf(offset)){
                        KeyPtrSet borrow = borrowData(nextLeftUnit->_tracks[offset], nextUnit->_tracks[0]);
                        if(borrow.getKey(0) == 0){
                            unsigned deleteIndex = nextUnit->_tracks[0].getMinIndex();
                            mergeNode(nextLeftUnit->_tracks[offset], nextUnit->_tracks[0]);
                            nextUnit = nextLeftUnit;
                            connectSideUnit(nextUnit);
                            deleteCurrentData(deleteIndex, offset);
                        }
                        else{
                            _tracks[i]._data[leftOffset].setKey(0, nextUnit->_tracks[0].getMinIndex());
                        }
                    }

                    nextUnit->deleteData(idx, offset);
                    if(_tracks[offset]._data[leftOffset].getKey(0) == idx){
                        _tracks[offset]._data[leftOffset].setKey(0, nextUnit->_tracks[0].getMinData().getKey(0));
                    }
                    return;
                }
                else if(nextUnit != nullptr){
                    nextUnit->deleteData(idx, offset);
                    return;
                }*/

                nextUnit->deleteData(idx, offset);
                return;
            }

            throw "Delete Fail";
        }
    }

    void deleteCurrentData(unsigned idx, unsigned unit_offset, unsigned data_offset = 0){
        if(isLeaf()){
            Unit *selfUnit = this;
            Unit *rightUnit = nullptr;
            Unit *leftUnit = nullptr;
            bool side = false;

            if(!selfUnit->_tracks[unit_offset].isLeftMostOffset(data_offset)){
                unsigned leftOffset = getParentUnit()->_tracks[unit_offset].getLeftMostOffset();
                leftUnit = (Unit *)getParentUnit()->_tracks[unit_offset]._data[leftOffset].getPtr();
            }

            if(!selfUnit->_tracks[unit_offset].isRightMostOffset(data_offset)){
                unsigned rightOffset = getParentUnit()->_tracks[unit_offset].getRightMostOffset();
                rightUnit = (Unit *)getParentUnit()->_tracks[unit_offset]._data[rightOffset].getPtr();
            }
            else{
                side = true;
                rightUnit = getParentUnit()->getSideUnit();
            }
            std::clog << "<log> rightUnit: " << rightUnit << std::endl;

            if(isHalf(unit_offset)){
                if(rightUnit != nullptr){
                    KeyPtrSet borrow = borrowData(selfUnit->_tracks[unit_offset], rightUnit->_tracks[0]);
                    std::clog << "<log> borrow: " << borrow << std::endl;
                    if(borrow.getKey(0) == 0){
                        unsigned deleteIndex = rightUnit->_tracks[0].getMinIndex();
                        mergeNode(selfUnit->_tracks[unit_offset], rightUnit->_tracks[0]);
                        if(side){
                            getParentUnit()->connectSideUnit(selfUnit);
                        }
                        getParentUnit()->deleteCurrentData(deleteIndex, unit_offset);
                    }
                    else{
                        _tracks[unit_offset]._data[data_offset].setKey(0, rightUnit->_tracks[0].getMinIndex());
                    }
                }
                
                if(_isRoot && isEmpty(0)){
                    _isRoot = false;
                    selfUnit->setRoot(true);
                    selfUnit->connectParentUnit(nullptr);
                }
            }
            _tracks[unit_offset].deleteData(idx);
        }
        else{
            
        }
    }

    KeyPtrSet borrowData(Node &source, Node& destination){
        
        if(source._isLeaf && !destination.isHalf()){
            KeyPtrSet borrow = destination.getMinData();
            destination.deleteData(destination.getMinIndex());
            source.insertData(borrow.getKey(0), borrow.getPtr());
            //std::clog << "<log> destination: " << destination << std::endl;

            return borrow;
        }
        else if(!source._isLeaf){
            throw "Internal Node Developing";
        }

        return KeyPtrSet();
    }

    // Merge right Node to left Node
    void mergeNode(Node &left, Node &right){
        std::clog << "<log> Merge" << std::endl;

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
    }

    void copyHalfNode(Node &source, Node &destination){
        if(source._isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(destination._bitmap[i]){
                    source.insertData(destination._data[i].getKey(0), destination._data[i].getPtr());
                    destination.deleteMark(i);
                }
            }
        }
        else{

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