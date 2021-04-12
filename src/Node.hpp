#ifndef NODE_H
#define NODE_H

#include "Options.hpp"
#include "KeyPtrSet.hpp"
#include "Unit.hpp"
#include "Counter.hpp"
#include <ostream>

namespace Evaluation{
    uint64_t log2(uint64_t number) {
        uint64_t count = 0;
        while (number >>= 1) ++count;
        return count;
    }

    uint64_t countSkyrmion(uint64_t data) {
        uint64_t number = 0;
        uint64_t a = 0;
        while (data > 0) {
            a = data % 2;
            data = data / 2;
            if (a == 1) {
                number++;
            }
        }
        return number;
    }

    void sequential_read(){

    }

    void range_read(){

    }

    void overwrite(){

    }

    void permute(){

    }

    void binary_search(){

    }

    void bit_binary_search(){

    }

};

struct Unit;

struct Node{
    Node(Options options = Options(), bool None = true, bool isLeaf = true) : _options(options){
        _data = nullptr;
        if(!None){
            _isLeaf = isLeaf;
            _sideBack = nullptr;
            _sideFront = nullptr;
            _parent = nullptr;
            _parentOffset = 0;
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
        _sideBackBitmap = right._sideBackBitmap;
        _sideFront = right._sideFront;
        _sideFrontBitmap = right._sideFrontBitmap;
        _parent = right._parent;
        _parentOffset = right._parentOffset;
        _id = right._id;
        _isValid = right._isValid;

        for(int i = 0; i < _options.track_length; ++i){
            _data[i] = right._data[i];
        }
    }

    ~Node(){
        delete[] _data;
    }

    Node &operator=(const Node &right){
        if(_data != nullptr)
            delete[] _data;

        _options = right._options;

        _data = new KeyPtrSet[_options.track_length]();

        _isLeaf = right._isLeaf;
        _sideBack = right._sideBack;
        _sideBackBitmap = right._sideBackBitmap;
        _sideFront = right._sideFront;
        _sideFrontBitmap = right._sideFrontBitmap;
        _parent = right._parent;
        _parentOffset = right._parentOffset;
        _id = right._id;
        _isValid = right._isValid;

        //_shiftCounter = right._shiftCounter;

        for(int i = 0; i < _options.track_length; ++i){
            _data[i] = right._data[i];
        }

        return *this;
    }

    /* Major Functions */

    //* Return array of KeyPtrSet
    KeyPtrSet *readData(uint64_t lower, uint64_t upper){
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
    void *searchData(uint64_t idx, uint64_t &next_unit_offset){
        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            if(_isLeaf){
                switch (_options.search_mode){
                case Options::search_function::SEQUENTIAL:
                    //TODO evaluation 
                    break;
                case Options::search_function::TRAD_BINARY_SEARCH:
                {
                    int l = 0;
                    int h = _options.track_length - 1;
                    while (l <= h) {
                        _shiftCounter.count(2 * _options.word_length);
                        _readCounter.count(_options.word_length);
                        int mid = (l + h) / 2;
                        if (_data[mid].getKey(0) > idx) {
                            h = mid - 1;
                        }
                        else if (_data[mid].getKey(0) < idx) {
                            l = mid + 1;
                        }
                        else{
                            break;
                        }
                    }
                }
                    break;
                case Options::search_function::BIT_BINARY_SEARCH:
                    _shiftCounter.count(2 * _options.word_length);
                    break;
                default:
                    throw "undefined search operation";
                }

                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && _data[i].getKey(0) == idx){
                        return _data[i].getPtr();
                    }
                }

                return nullptr;
            }
            else{
                switch (_options.search_mode)
                {
                case Options::search_function::SEQUENTIAL:
                    //TODO evaluation 
                    break;
                case Options::search_function::TRAD_BINARY_SEARCH:
                    _shiftCounter.count(2 * _options.word_length * Evaluation::log2(_options.track_length));
                    break;
                case Options::search_function::BIT_BINARY_SEARCH:
                    _shiftCounter.count(2 * _options.word_length);
                    break;
                default:
                    throw "undefined search operation";
                }

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
    void updateData(uint64_t idx, uint64_t data){
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

    void updateIndex(uint64_t offset, uint64_t idx){
        _data[offset].setKey(0, idx);
    }

    /**
     * * Insert KeyPtrSet
     * TODO evaluation
     * TODO insert into the same pointer, just add the index to the key-point set
     * @param split if false, data will direct insert
    */
    void insertData(uint64_t idx, void *data, Offset &insertPosition, bool split = true){
        //// evaluate for find a insert position
        /*
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
        */

        if(_isLeaf){
            //* Evaluate for insert a data
            switch (_options.update_mode){
            case Options::update_function::OVERWRITE:
                _shiftCounter.count(2 * _options.word_length);
                _removeCounter.count(2 * _options.word_length);
                _insertCounter.count(Evaluation::countSkyrmion(idx));
                _insertCounter.count(Evaluation::countSkyrmion((uint64_t)data));
                _shiftCounter.count(2 * _options.word_length);
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

            KeyPtrSet newData(2);
            newData.setPtr(data);
            newData.addKey(idx);

            bool insertSide = false;
            uint64_t shiftPos = getShiftPosition();
            insertPosition = getInsertPosition(idx, insertSide);

            if(_options.node_ordering == Options::ordering::SORTED){
                if(_data[insertPosition].getBitmap(0)){
                    if(shiftPos < insertPosition){
                        if(!insertSide)
                            insertPosition -= 1;
                        for(int i = shiftPos; i < insertPosition; ++i){
                            _data[i] = _data[i+1];
                            _shiftCounter.count(2 * _options.word_length);
                        }
                    }
                    else{
                        for(int i = shiftPos; i > insertPosition; --i){
                            _data[i] = _data[i-1];
                            _shiftCounter.count(2 * _options.word_length);
                        }
                    }
                }

                _data[insertPosition] = newData;
            }
            else if(_options.node_ordering == Options::ordering::UNSORTED){
                _data[insertPosition] = newData;
            }
            else{
                throw "undefined insert operation";
            }
        }
        else{
            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                //* Evaluate for insert a data
                switch (_options.update_mode){
                case Options::update_function::OVERWRITE:
                    _shiftCounter.count(2 * _options.word_length);
                    _removeCounter.count(2 * _options.word_length);
                    _insertCounter.count(Evaluation::countSkyrmion(idx));
                    _insertCounter.count(Evaluation::countSkyrmion((uint64_t)data));
                    _shiftCounter.count(2 * _options.word_length);
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

                KeyPtrSet newData(2, false);
                newData.setPtr(data);
                newData.addKey(idx);

                bool insertSide = false;
                uint64_t shiftPos = getShiftPosition();
                insertPosition = getInsertPosition(idx, insertSide);

                //* Only insert at unused KeyPtrSet
                if(_data[insertPosition].getBitmap(0)){
                    if(shiftPos < insertPosition){
                        if(!insertSide){
                            insertPosition -= 1;
                        }
                        for(int i = shiftPos; i < insertPosition; ++i){
                            _data[i] = _data[i+1];
                            _shiftCounter.count(2 * _options.word_length);
                        }
                    }
                    else{
                        for(int i = shiftPos; i > insertPosition; --i){
                            _data[i] = _data[i-1];
                            _shiftCounter.count(2 * _options.word_length);
                        }
                    }
                }

                _data[insertPosition] = newData;  

                if(_sideBack != nullptr){
                    if(insertSide){
                        void *temp = _sideBack;
                        _sideBack = (Unit *)_data[insertPosition].getPtr();
                        _data[insertPosition].setPtr(temp);
                    }
                    else if(!insertSide && split){
                        int i = 0;
                        while(!_data[insertPosition+i+1].getBitmap(0))++i;
                        void *temp = _data[insertPosition+i+1].getPtr();
                        _data[insertPosition+i+1].setPtr(_data[insertPosition].getPtr());
                        _data[insertPosition].setPtr(temp);
                    }
                }
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                std::clog << "<log> <Node::insertData()> idx: " << idx << std::endl;
                std::clog << "<log> <Node::insertData()> data: " << data << std::endl;
                std::clog << "<log> <Node::insertData()> _sideBack: " << _sideBack << std::endl;
                //* insert into the same pointer, just add the index to the key-point set
                for(int i = 0; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && _data[i].getPtr() == data){
                        _data[i].addKey(idx);
                        
                        for(int i = 1; i < _options.track_length; ++i){
                            if(_data[i].getBitmap(0) && _data[i-1].getBitmap(1) && _data[i].getKey(0) < _data[i-1].getKey(1)){
                                uint64_t temp = _data[i].getKey(0);
                                _data[i].setKey(0, _data[i-1].getKey(1));
                                _data[i-1].setKey(1, temp);
                            } 
                        }
                        std::clog << "<log> <Node::insertData()> end with same pointer" << std::endl;
                        return;
                    }
                }

                KeyPtrSet newData(_options.kp_length, false);
                newData.setPtr(data);
                newData.addKey(idx);

                bool insertSide = false;
                uint64_t shiftPos = getShiftPosition();
                insertPosition = getInsertPosition(idx, insertSide);

                std::clog << "<log> <Node::insertData()> insertSide: " << insertSide << std::endl;

                /*
                if(insertSide && getRightMostOffset() != -1 && _data[getRightMostOffset()].getSize() == 1){
                    _data[getRightMostOffset()].addKey(idx);
                    _sideBack = (Unit *)data;
                    return;
                }
                */

                //* Only insert at unused KeyPtrSet
                if(_data[insertPosition].getBitmap(0)){
                    if(shiftPos < insertPosition){
                        if(!insertSide){
                            insertPosition -= 1;
                        }
                        for(int i = shiftPos; i < insertPosition; ++i){
                            _data[i] = _data[i+1];
                        }
                    }
                    else{
                        for(int i = shiftPos; i > insertPosition; --i){
                            _data[i] = _data[i-1];
                        }
                    }
                }
                /*
                uint64_t rightMostOffset = getRightMostOffset();
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
                        uint64_t temp = newData.getKey(0);
                        newData.setKey(0, _data[insertPos-1].getKey(1));
                        _data[insertPos-1].setKey(1, temp);
                    }
                    _data[insertPos] = newData;
                }   
                */
                //if(getSize() == 1 && _data[0].getSize() < 2){
                    
               //     return;
                //}

                
                _data[insertPosition] = newData;

                
                for(int i = 1; i < _options.track_length; ++i){
                    if(_data[i].getBitmap(0) && _data[i-1].getBitmap(1) && _data[i].getKey(0) < _data[i-1].getKey(1)){
                        uint64_t temp = _data[i].getKey(0);
                        _data[i].setKey(0, _data[i-1].getKey(1));
                        _data[i-1].setKey(1, temp);
                    } 
                }
                
                /*
                if(_data[getRightMostOffset()].getSize() == 1){
                    _sideBack = (Unit *)_data[getRightMostOffset()].getPtr();
                }
                */

               if(_sideBack != nullptr){
                    if(insertSide && split){
                        void *temp = _sideBack;
                        _sideBack = (Unit *)_data[insertPosition].getPtr();
                        _data[insertPosition].setPtr(temp);
                    }
                    else if(!insertSide && insertPosition == 0 && split){
                        int i = 0;
                        while(!_data[insertPosition+i+1].getBitmap(0))++i;
                        //if(_data[insertPosition+i+1].getSize() == 1){
                            void *temp = _data[insertPosition+i+1].getPtr();
                            _data[insertPosition+i+1].setPtr(_data[insertPosition].getPtr());
                            _data[insertPosition].setPtr(temp);
                        //}
                        /*
                        if(_data[insertPosition+i+1].getSize() == 1){
                            void *temp = _data[insertPosition+i+1].getPtr();
                            _data[insertPosition+i+1].setPtr(_data[insertPosition].getPtr());
                            _data[insertPosition].setPtr(temp);
                        }
                        else{
                            std::clog << "<log> <Node::insertData()> idx: " << idx << std::endl;
                            //throw "test";
                        }*/

                        if(_data[insertPosition+i+1].getSize() == _options.unit_size){
                            _data[insertPosition].addKey(_data[insertPosition+i+1].getKey(0));
                            _data[insertPosition+i+1].delKey(0);
                        }
                    }
                }

                /*
                if(_sideBack != nullptr){
                    if(!insertSide && split && insertPos == 0){
                        int i = 0;
                        while(!_data[insertPos+i+1].getBitmap(0))++i;
                        void *temp = _data[insertPos+i+1].getPtr();
                        _data[insertPos+i+1].setPtr(_data[insertPos].getPtr());
                        _data[insertPos].setPtr(temp);
                    }
                }
                */
               std::clog << "<log> <Node::insertData()> end with new pointer" << std::endl;
            }
    
        }
    }

    /**
     * * Insert KeyPtrSet to front
    */
    void insertDataFromFront(KeyPtrSet data){
        uint64_t shiftPos = _options.track_length - 1;
        uint64_t insertPos = 0;

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
        uint64_t insertPos = getSize();
        _data[insertPos] = data;
    }

    /**
     * * Find and delete index 
     * TODO evaluation 
     * TODO general unit parameter 
     * TODO redesign as deleteData(offset, side) to direct delete (still need to check right most) 
     * @param side for internal, if true, and try to delete the right most index, that will delete the side unit
     */
    void deleteData(uint64_t idx, bool side = false){
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
    void connectParentNode(Unit *unit, uint64_t offset = 0){
        _parent = unit;
        _parentOffset = offset;
    }

    /**
     * @param side ?
    */
    void deleteMark(uint64_t offset, bool side = false){
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
    uint64_t getOffsetByIndex(uint64_t idx){
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
    uint64_t getMinIndex(){
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
    uint64_t getMaxIndex(){
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
    uint64_t getLeftMostOffset(){
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
    uint64_t getRightMostOffset(){
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
    bool isRightMostOffset(uint64_t offset) const{
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
    bool isLeftMostOffset(uint64_t offset) const{
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
    uint64_t getClosestRightOffset(uint64_t offset){
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
    uint64_t getClosestLeftOffset(uint64_t offset){
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
    uint64_t getInsertPosition(uint64_t wait_insert_idx, bool &insertSide){
        if(_isLeaf){
            if(_options.node_ordering == Options::ordering::SORTED){
                uint64_t last = 0;
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
            uint64_t last = 0;
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

    uint64_t getShiftPosition(){
        uint64_t shiftPoint = -1;

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
           std::clog << "<log> <Node::getShiftPosition()> " << std::endl;
            for(int i = 0; i < _options.track_length; ++i){
                std::clog << _data[i].getKey(0) << " ";
                std::clog << _data[i].getBitmap(0) << " ";
                std::clog << _data[i].getKey(1) << " ";
                std::clog << _data[i].getBitmap(1) << ", ";
                if(!_data[i].getBitmap(0)){
                    shiftPoint = i;
                    break;
                }
            }
            std::clog << std::endl;
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
    uint64_t getSize() const{
        uint64_t counter = 0;

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
    uint64_t _parentOffset;
    bool _isValid;

    /* System */
    Options _options;
    uint64_t _id;

    Counter _readCounter = Counter("Read");
    Counter _shiftCounter = Counter("Shift");
    Counter _insertCounter = Counter("Insert");
    Counter _removeCounter = Counter("Remove");
    Counter _migrateCounter = Counter("Migrate");
};

std::ostream &operator<<(std::ostream &out, const Node &right){
    //std::clog << "<log> Node Print" << std::endl;
    

    //* status
    // out << "Node "<< right._id << "\n";
    // out << right._shiftCounter << "\n";
    // out << right._insertCounter << "\n";
    // out << right._removeCounter << "\n";
    // out << right._readCounter << "\n";
    // out << right._migrateCounter << "\n";

    //* debug
    out << "\n\t\t(\n";
    // out << "\t\t\t" << "Node "<< right._id << "\n";
    // out << "\t\t\t" << right._shiftCounter << "\n";
    // out << "\t\t\t" << right._insertCounter << "\n";
    // out << "\t\t\t" << right._removeCounter << "\n";
    // out << "\t\t\t" << right._readCounter << "\n";
    // out << "\t\t\t" << right._migrateCounter << "\n";

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