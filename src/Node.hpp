#ifndef NODE_H
#define NODE_H

#include "Options.hpp"
#include "KeyPtrSet.hpp"
#include "Unit.hpp"
#include "Counter.hpp"
#include <ostream>

struct Unit;

struct Node{
    Node(Options options = Options(), bool None = true, bool isLeaf = true) : _options(options){
        if(!None){
            _isLeaf = isLeaf;
            _sideBack = nullptr;
            _sideFront = nullptr;
            _parent = nullptr;
            _sideBackBitmap = false;
            _sideFrontBitmap = false;
            _isValid = false;
            
            _data = new KeyPtrSet[_options.track_length]();
            if(!isLeaf){
                for(int i = 0; i < _options.track_length; ++i){
                    _data[i] = KeyPtrSet(_options.kp_length);
                }
            }

            _id = NodeId++;
        }
    }

    Node(const Node &right){
        _options = right._options;

        _data = new KeyPtrSet[_options.track_length]();

        _isLeaf = right._isLeaf;
        _sideBack = right._sideBack;
        _parent = right._parent;
        _id = right._id;
        _sideBackBitmap = right._sideBackBitmap;
        _isValid = right._isValid;

        for(int i = 0; i < _options.track_length; ++i){
            _data[i] = right._data[i];
        }
    }

    ~Node(){
        delete[] _data;
    }

    Node &operator=(const Node &right){
        _options = right._options;

        _data = new KeyPtrSet[_options.track_length]();

        _isLeaf = right._isLeaf;
        _sideBack = right._sideBack;
        _parent = right._parent;
        _id = right._id;
        _sideBackBitmap = right._sideBackBitmap;
        _isValid = right._isValid;

        //_shiftCounter = right._shiftCounter;

        for(int i = 0; i < _options.track_length; ++i){
            _data[i] = right._data[i];
        }

        return *this;
    }

    /* Major Functions */

    //* Return array of KeyPtrSet
    KeyPtrSet *readData(unsigned lower, unsigned upper){
        if(upper < lower)
            throw "invalid read bound";
         
        // calculate performance
        switch (_options.read_mode)
        {
        case Options::read_function::SEQUENTIAL:
            //TODO evaluation 
            break;
        case Options::read_function::RANGE_READ:
            //TODO evaluation 
            break;
        default:
            throw "undefined read operation";
            break;
        }

        KeyPtrSet *arr = new KeyPtrSet[upper - lower + 1];
        for(int i = lower; i < upper; ++i){
            arr[i] = _data[i];
        }
        return arr; 
    }

    /**
     * TODO evaluation
     */
    void *searchData(unsigned idx, unsigned &next_unit_offset){
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

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            if(_isLeaf){
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && _data[i].getKey(0) == idx){
                        return _data[i].getPtr();
                    }
                }

                return nullptr;
            }
            else{
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && idx < _data[i].getKey(0)){
                        return _data[i].getPtr();
                    }
                }
                
            }

            return _sideBack;
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            if(_isLeaf){
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && _data[i].getKey(0) == idx){
                        return _data[i].getPtr();
                    }
                }

                return nullptr;
            }
            else{
                for(int i = 0; i < _options.track_length; ++i){
                    for(int j = 0; j < _options.unit_size; ++j){
                        if(_data[i].getBitmap(j) && idx < _data[i].getKey(j)){
                            next_unit_offset = j;
                            return _data[i].getPtr();
                        }
                    } 
                }

                next_unit_offset = _options.unit_size;
                return _sideBack;      
            }
            
        }

        
    }
    
    //?
    void updateData(unsigned idx, unsigned data){
        switch (_options.update_mode)
        {
        case Options::update_function::OVERWRITE:
            //TODO evaluation 
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

    void updateIndex(unsigned offset, unsigned idx){
        _data[offset].setKey(0, idx);
    }

    /**
     * * Insert KeyPtrSet
     * TODO evaluation
     * TODO insert into the same pointer, just add the index to the key-point set
     * @param split if false, data will direct insert
    */
    void insertData(unsigned idx, void *data, bool split = true){
        // evaluate for find a insert position
        switch (_options.insert_mode)
        {
        case Options::insert_function::SEQUENTIAL:
            //TODO evaluation 
            break;
        case Options::insert_function::BIT_BINARY_INSERT:
            //TODO evaluation 
            break;
        default:
            throw "undefined insert operation";
        }

        // evaluate for insert a data
        switch (_options.update_mode)
        {
        case Options::update_function::OVERWRITE:
            //TODO evaluation 
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

        if(_isLeaf){
            KeyPtrSet newData(2);
            newData.setPtr(data);
            newData.addKey(idx);

            bool insertSide = false;
            unsigned shiftPos = getShiftPosition();
            unsigned insertPos = getInsertPosition(idx, insertSide);

            switch (_options.node_ordering){
                case Options::ordering::SORTED:
                {
                    if(_data[insertPos].getBitmap(0)){
                        if(shiftPos < insertPos){
                            if(!insertSide)
                                insertPos -= 1;
                            for(int i = shiftPos; i < insertPos; ++i){
                                _data[i] = _data[i+1];
                            }
                        }
                        else{
                            for(int i = shiftPos; i > insertPos; --i){
                                _data[i] = _data[i-1];
                            }
                        }
                    }

                    _data[insertPos] = newData;
                }
                    break;
                case Options::ordering::UNSORTED:
                    _data[insertPos] = newData;
                    break;
                default:
                    throw "undefined insert operation";
            }
        }
        else{
            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                KeyPtrSet newData(2, false);
                newData.setPtr(data);
                newData.addKey(idx);

                bool insertSide = false;
                unsigned shiftPos = getShiftPosition();
                unsigned insertPos = getInsertPosition(idx, insertSide);

                //* Only insert at unused KeyPtrSet
                if(_data[insertPos].getBitmap(0)){
                    if(shiftPos < insertPos){
                        if(!insertSide){
                            insertPos -= 1;
                        }
                        for(int i = shiftPos; i < insertPos; ++i){
                            _data[i] = _data[i+1];
                        }
                    }
                    else{
                        for(int i = shiftPos; i > insertPos; --i){
                            _data[i] = _data[i-1];
                        }
                    }
                }

                _data[insertPos] = newData;  

                if(_sideBack != nullptr){
                    if(insertSide){
                        void *temp = _sideBack;
                        _sideBack = (Unit *)_data[insertPos].getPtr();
                        _data[insertPos].setPtr(temp);
                    }
                    else if(!insertSide && split){
                        int i = 0;
                        while(!_data[insertPos+i+1].getBitmap(0))++i;
                        void *temp = _data[insertPos+i+1].getPtr();
                        _data[insertPos+i+1].setPtr(_data[insertPos].getPtr());
                        _data[insertPos].setPtr(temp);
                    }
                }
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                std::clog << "<log> <Node::insertData()> _sideBack: " << _sideBack << std::endl;
                std::clog << "<log> <Node::insertData()> data: " << data << std::endl;
                if(_sideBack == data){

                }
                //* insert into the same pointer, just add the index to the key-point set
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && _data[i].getPtr() == data){
                        _data[i].addKey(idx);
                        
                        if(i > 0 && _data[i].getKey(0) < _data[i-1].getKey(1)){
                            unsigned temp = _data[i].getKey(0);
                            _data[i].setKey(0, _data[i-1].getKey(1));
                            _data[i-1].setKey(1, temp);
                        } 
                        
                        return;
                    }
                }

                KeyPtrSet newData(_options.kp_length, false);
                newData.setPtr(data);
                newData.addKey(idx);

                bool insertSide = false;
                unsigned shiftPos = getShiftPosition();
                unsigned insertPos = getInsertPosition(idx, insertSide);

                std::clog << "<log> <Node::insertData()> insertSide: " << insertSide << std::endl;
                if(insertSide && getRightMostOffset() != -1 && _data[getRightMostOffset()].getSize() == 1){
                    _data[getRightMostOffset()].addKey(idx);
                    _sideBack = (Unit *)data;
                    return;
                }

                //* Only insert at unused KeyPtrSet
                if(_data[insertPos].getBitmap(0)){
                    if(shiftPos < insertPos){
                        if(!insertSide){
                            insertPos -= 1;
                        }
                        for(int i = shiftPos; i < insertPos; ++i){
                            _data[i] = _data[i+1];
                        }
                    }
                    else{
                        for(int i = shiftPos; i > insertPos; --i){
                            _data[i] = _data[i-1];
                        }
                    }
                }
                /*
                unsigned rightMostOffset = getRightMostOffset();
                if(rightMostOffset != -1 && insertSide && _data[rightMostOffset].getSize() == 1){
                    _data[rightMostOffset].addKey(idx);
                    _sideBack = (Unit *)data;
                    if(insertPos != rightMostOffset){
                        deleteMark(insertPos);
                    }
                    return;
                }
                else{
                    if(insertPos > 0 && newData.getKey(0) < _data[insertPos-1].getKey(1)){
                        unsigned temp = newData.getKey(0);
                        newData.setKey(0, _data[insertPos-1].getKey(1));
                        _data[insertPos-1].setKey(1, temp);
                    }
                    _data[insertPos] = newData;
                }   
                */
                //if(getSize() == 1 && _data[0].getSize() < 2){
                    
               //     return;
                //}

                
                _data[insertPos] = newData;

                for(int i = 1; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && _data[i].getKey(0) < _data[i-1].getKey(1)){
                        unsigned temp = _data[i].getKey(0);
                        _data[i].setKey(0, _data[i-1].getKey(1));
                        _data[i-1].setKey(1, temp);
                    } 
                }
                
                if(_data[getRightMostOffset()].getSize() == 1){
                    _sideBack = (Unit *)_data[getRightMostOffset()].getPtr();
                }

                if(_sideBack != nullptr){
                    if(!insertSide && split && insertPos == 0){
                        int i = 0;
                        while(!_data[insertPos+i+1].getBitmap(0))++i;
                        void *temp = _data[insertPos+i+1].getPtr();
                        _data[insertPos+i+1].setPtr(_data[insertPos].getPtr());
                        _data[insertPos].setPtr(temp);
                    }
                }

            }
    
        }
    }

    /**
     * * Insert KeyPtrSet to front
    */
    void insertDataFromFront(KeyPtrSet data){
        unsigned shiftPos = _options.track_length - 1;
        unsigned insertPos = 0;

        //* Only insert at unused KeyPtrSet
        if(_data[insertPos].getBitmap(0)){
            if(shiftPos < insertPos){
                for(int i = shiftPos; i < insertPos; ++i){
                    _data[i] = _data[i+1];
                }
            }
            else{
                for(int i = shiftPos; i > insertPos; --i){
                    _data[i] = _data[i-1];
                }
            }
        }

        _data[insertPos] = data;
    }

    /**
     * * Insert KeyPtrSet to back
    */
    void insertDataFromBack(KeyPtrSet data){
        unsigned insertPos = getSize();
        _data[insertPos] = data;
    }

    /**
     * * Find and delete index 
     * TODO evaluation 
     * TODO general unit parameter 
     * TODO redesign as deleteData(offset, side) to direct delete (still need to check right most) 
     * @param side for internal, if true, and try to delete the right most index, that will delete the side unit
     */
    void deleteData(unsigned idx, bool side = false){
        std::clog << "<log> <Node::deleteData()>: " << idx << std::endl;

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

        /*
        switch (_options.delete_mode)
        {
        case Options::delete_function::SEQUENTIAL:
            break;
        case Options::delete_function::BALANCE:
            break;
        default:
            throw "undefined operation";
        }*/

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0) && idx == _data[i].getKey(0)){ //TODO
                    // delete side unit
                    if(!_isLeaf && isRightMostOffset(i) && side){
                        connectBackSideUnit((Unit *)_data[i].getPtr());
                    }
                    deleteMark(i);
                    return;
                }
            }
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            if(_isLeaf){
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && idx == _data[i].getKey(0)){
                        _data[i].delKey(0);
                        return;
                    }
                }
            }
            else{
                for(int i = 0; i < _options.track_length; ++i){
                    for(int j = 0; j < _options.unit_size; ++j){
                        if(_data[i].getBitmap(j) && idx == _data[i].getKey(j)){
                            _data[i].delKey(j);
                            return;
                        }
                    }
                }
            }
            
        }

        throw "delete not found";
    }

    /**
     * * Delete KeyPtrSet from front
    */
    void deleteDataFromFront(){

    }

    /**
     * * Delete KeyPtrSet from back
    */
    void deleteDataFromBack(){

    }

    /* Minor Functions */

    /**
     * * Done
    */
    void connectBackSideUnit(Unit *unit){
        _sideBack = unit;
        _sideBackBitmap = true;
    }

    /**
     * * Done
    */
    void connectFrontSideUnit(Unit *unit){
        _sideFront = unit;
        _sideFrontBitmap = true;
    }

    /**
     * * Done
    */
    void connectParentNode(Unit *unit, unsigned offset = 0){
        _parent = unit;
        _parentOffset = offset;
    }

    /**
     * @param side ?
    */
    void deleteMark(unsigned offset, bool side = false){
        if(_isLeaf){
            //_data[offset].delKey(0);
            _data[offset].delAll();
        }
        else{
            //_data[offset / _options.unit_size].delKey(offset % _options.unit_size);
            //_data[offset].delKey(0);
            _data[offset].delAll();
        }
    }

    /**
     * * Done
     * @return offset
    */
    unsigned getOffsetByIndex(unsigned idx){
        if(_isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0) && idx == _data[i].getKey(0)){
                    return i;
                }
            }
        }
        else{
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(_data[i].getBitmap(j) && idx == _data[i].getKey(j)){
                        return i + j;
                    }
                }
            }
        }
        
        return _options.track_length * _options.unit_size;
    }

    /**
     * * Done
    */
    unsigned getMinIndex(){
        if(_isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0)){
                    return _data[i].getKey(0);
                }
            }
        }
        else{
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(_data[i].getBitmap(j)){
                        return _data[i].getKey(j);
                    }
                }
            }
        }
        
        throw "getMinIndex() error";
    }

    /**
     * * Done
    */
    unsigned getMaxIndex(){
        if(_isLeaf){
            for(int i = _options.track_length - 1; i >= 0; --i){
                if(_data[i].getBitmap(0)){
                    return _data[i].getKey(0);
                }
            }
        }
        else{
            for(int i = _options.track_length - 1; i >= 0; --i){
                for(int j = _options.unit_size - 1; j >= 0; --j){
                    if(_data[i].getBitmap(j)){
                        return _data[i].getKey(j);
                    }
                }
            }
        }
        
    }

    KeyPtrSet getMinData(){
        if(_isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0)){
                    return _data[i];
                }
            }
        }
        else{
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(_data[i].getBitmap(j)){
                        return _data[i]; //?
                    }
                }
            }
        }
        
    }

    KeyPtrSet getMaxData(){
        if(_isLeaf){
            for(int i = _options.track_length - 1; i >= 0; --i){
                if(_data[i].getBitmap(0)){
                    return _data[i];
                }
            }
        }
        else{
            for(int i = _options.track_length - 1; i >= 0; --i){
                for(int j = _options.unit_size - 1; j >= 0; --j){
                    if(_data[i].getBitmap(j)){
                        return _data[i]; //?
                    }
                }
            }
        }
        
    }

    /**
     * * Done
    */
    unsigned getLeftMostOffset(){
        if(_isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0))
                    return i;
            }
        }
        else{
            /*
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(_data[i].getBitmap(j))
                        return i + j;
                }
            }*/

            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0))
                    return i;
            }
        }

        return -1;
    }

    /**
     * * Done
    */
    unsigned getRightMostOffset(){
        if(_isLeaf){
            for(int i = _options.track_length - 1; i >= 0; --i){
                if(_data[i].getBitmap(0))
                    return i;
            }
        }
        else{
            /*
            for(int i = _options.track_length - 1; i >= 0; --i){
                for(int j = _options.unit_size - 1; j >= 0 ; --j){
                    if(_data[i].getBitmap(j))
                        return i + j;
                }
            }
            */
            for(int i = _options.track_length - 1; i >= 0; --i){
                if(_data[i].getBitmap(0))
                    return i;
            }
        }

        return -1;
    }

    /**
     * ? Checking
    */
    bool isRightMostOffset(unsigned offset) const{
        if(offset == _options.track_length)
            return true;
            
        if(offset == -1)
            return false;

        if(_isLeaf){
            if(!_data[offset].getBitmap(0))
                return false;

            for(int i = offset + 1; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0))
                    return false;
            }
        }
        else{
            if(!_data[offset / _options.unit_size].getBitmap(offset % _options.unit_size))
                return false;

            for(int i = offset + 1; i < _options.track_length; ++i){
                if(_data[i / _options.unit_size].getBitmap(i % _options.unit_size))
                    return false;
            }
        }

        return true;
    }

    /**
     * ? Checking
    */
    bool isLeftMostOffset(unsigned offset) const{
        if(offset == -1 || offset == _options.track_length)
            return false;
        
        if(_isLeaf){
            if(!_data[offset].getBitmap(0))
                return false;

            for(int i = 0; i < offset; ++i){
                if(_data[i].getBitmap(0))
                    return false;
            }
        }
        else{
            if(!_data[offset / _options.unit_size].getBitmap(offset % _options.unit_size))
                return false;

            for(int i = 0; i < offset; ++i){
                if(_data[i / _options.unit_size].getBitmap(i % _options.unit_size))
                    return false;
            }
        }
        //std::clog << "<log> offset: " << offset << std::endl;
        //std::clog << "<log> _options.unit_size: " << _options.unit_size << std::endl;

        return true;
    }

    /**
     * ? Checking
    */
    unsigned getClosestRightOffset(unsigned offset){
        if(_isLeaf){
            for(int i = offset + 1; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0)){
                    return i;
                }
            }
        }
        else{
            for(int i = offset + 1; i < _options.track_length; ++i){
                if(_data[i / _options.unit_size].getBitmap(i % _options.unit_size)){
                    return i;
                }
            }
        }
        

        throw "getClosestRightOffset() fail";
    }

    /**
     * ? Checking
    */
    unsigned getClosestLeftOffset(unsigned offset){
        if(_isLeaf){
            for(int i = offset - 1; i >= 0; --i){
                if(_data[i].getBitmap(0)){
                    return i;
                }
            }
        }
        else{
            for(int i = offset - 1; i >= 0; --i){
                if(_data[i / _options.unit_size].getBitmap(i % _options.unit_size)){
                    return i;
                }
            }
        }

        throw "getClosestLeftOffset() fail";
    }
    
    // Return insert position
    unsigned getInsertPosition(unsigned wait_insert_idx, bool &insertSide){
        if(_isLeaf){
            if(_options.node_ordering == Options::ordering::SORTED){
                unsigned last = 0;
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0)){
                        last = i + 1;
                    } 
                }
                
                for(int i = 0; i < last; ++i){
                    if(!_data[i].getBitmap(0) && _data[i+1].getBitmap(0) && wait_insert_idx < _data[i+1].getKey(0)){
                        return i;
                    }
                    else if(_data[i].getBitmap(0) && wait_insert_idx < _data[i].getKey(0)){
                        return i;
                    }
                }
                
                insertSide = true;
                return last == _options.track_length ? last - 1 : last;
            }
            else if(_options.node_ordering == Options::ordering::UNSORTED){
                for(int i = 0; i < _options.track_length; ++i){
                    if(!_data[i].getBitmap(0)){ //TODO
                        return i;
                    }
                }
                throw "full";
            }
            else{
                throw "undefined insert operation";
            }
        }
        else{
            unsigned last = 0;
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0)){
                    last = i + 1;
                }
            }
            
            for(int i = 0; i < last; ++i){
                if(!_data[i].getBitmap(0) && _data[i+1].getBitmap(0) && wait_insert_idx < _data[i+1].getKey(0)){
                    return i;
                }
                else if(_data[i].getBitmap(0) && wait_insert_idx < _data[i].getKey(0)){
                    return i;
                }
            }

            insertSide = true;
            return last == _options.track_length ? last - 1 : last;
        }
        
    }

    unsigned getShiftPosition(){
        unsigned shiftPoint = -1;

        if(_isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(!_data[i].getBitmap(0)){
                    shiftPoint = i;
                    break;
                }
            }
        }
        else{
            /*
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(!_data[i].getBitmap(j)){
                        shiftPoint = i + j;
                        break;
                    }
                }
            }
            */
            for(int i = 0; i < _options.track_length; ++i){
                if(!_data[i].getBitmap(0)){
                    shiftPoint = i;
                    break;
                }
            }
        }
        
        if(shiftPoint == -1)
            throw "full";
        
        return shiftPoint;
    }

    /**
     * * Done
    */
    bool isFull() const{
        if(_isLeaf){
            return getSize() == _options.track_length;
        }
        else{
            return getSize() == _options.track_length;
        }
        
    }

    /**
     * * Done
    */
    bool isHalf() const{
        return getSize() == _options.track_length / 2;
    }

    /**
     * * Done
    */
    void setValid(bool valid){
        _isValid = valid;
    }

    /**
     * * Done
    */
    bool isValid() const{
        return _isValid;
    }

    /**
     * * Done
    */
    unsigned getSize() const{
        unsigned counter = 0;

        if(_isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0)){
                    counter++;
                }
            }
        }
        else{
            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0)){
                        counter++;
                    }
                }
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                /*
                for(int i = 0; i < _options.track_length; ++i){
                    for(int j = 0; j < _options.unit_size; ++j){
                        if(_data[i].getBitmap(j)){
                            counter++;
                        }
                    }
                }*/
                
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) || _data[i].getBitmap(1)){
                        counter++;
                    }
                }
                
            }
            
        }

        return counter;
    }

    /* Data Member (Data) */
    KeyPtrSet *_data;

    /* Data Member (Meta Data) */
    bool _isLeaf;
    Unit *_sideBack;
    Unit *_sideFront;
    bool _sideBackBitmap;
    bool _sideFrontBitmap;
    Unit *_parent;
    unsigned _parentOffset;
    bool _isValid;

    /* System */
    Options _options;
    unsigned _id;

    Counter _readCounter = Counter("Read");
    Counter _shiftCounter = Counter("Shift");
    Counter _insertCounter = Counter("Insert");
    Counter _removeCounter = Counter("Remove");
    Counter _migrateCounter = Counter("Migrate");
};

std::ostream &operator<<(std::ostream &out, const Node &right){
    //std::clog << "<log> Node Print" << std::endl;
    out << "\n\t\t(\n";

    //* status
    out << "\t\t\t" << right._shiftCounter << "\n";

    out << "\t\t\t";
    bool first = true;
    for(int i = 0; i < right._options.track_length; ++i){
        if(first)first = false;
        else out << ", ";
        out << right._data[i];
    }
    out << " _sideFront: " << right._sideFront;
    if(!right._sideFrontBitmap) out << "*";
    out << " _sideBack: " << right._sideBack;
    if(!right._sideBackBitmap) out << "*";

    out << "\n\t\t)";

    if(!right.isValid()) out << "*";

    return out;
}

#endif