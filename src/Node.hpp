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
            _data = new KeyPtrSet[_options.track_length]();
            //_bitmap = new bool[_options.track_length]();
            _isLeaf = isLeaf;
            _side = nullptr;
            _parent = nullptr;
            _id = NodeId++;
            _sideBitmap = false;
            _isValid = true;
            if(!isLeaf){
                for(int i = 0; i < _options.track_length; ++i){
                    _data[i] = KeyPtrSet(_options.kp_length);
                }
            }
        }
    }

    Node(const Node &right){
        _options = right._options;

        _data = new KeyPtrSet[_options.track_length]();

        _isLeaf = right._isLeaf;
        _side = right._side;
        _parent = right._parent;
        _id = right._id;
        _sideBitmap = right._sideBitmap;
        _isValid = right._isValid;

        for(int i = 0; i < _options.track_length; ++i){
            _data[i] = right._data[i];
        }
    }

    Node &operator=(const Node &right){
        _options = right._options;

        _data = new KeyPtrSet[_options.track_length]();

        _isLeaf = right._isLeaf;
        _side = right._side;
        _parent = right._parent;
        _id = right._id;
        _sideBitmap = right._sideBitmap;
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

        if(_isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                _shiftCounter.count(2 * _options.word_length); //* evaluation key shifting
                if(_data[i].getBitmap(0) && _data[i].getKey(0) == idx){
                    _shiftCounter.count(2 * _options.word_length); //* evaluation pointer shifting

                    return _data[i].getPtr();
                }
            }
            throw -1;
        }
        else{
            for(int i = 0; i < _options.track_length; ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    _shiftCounter.count(2 * _options.word_length); //* evaluation key shifting
                    if(_data[i].getBitmap(j) && idx < _data[i].getKey(j)){
                        _shiftCounter.count(2 * _options.word_length); //* evaluation pointer shifting

                        next_unit_offset = j;
                        return _data[i].getPtr();
                    }
                } 
            }

            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                return _side;
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                KeyPtrSet last = getMaxData();
                if(last.getSize() == 1){
                    next_unit_offset = 0;
                    return _side;
                }
                else{
                    
                    next_unit_offset = 0;
                    return _side;
                }
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
            //TODO insert into the same pointer, just add the index to the key-point set
            for(int i = 0; i < _options.track_length; ++i){
                std::clog << "<log> _data[i]._size: " << _data[i]._size << std::endl;
                std::clog << "<log> _data[i].getPtr(): " << _data[i].getPtr() << std::endl;
                std::clog << "<log> _side: " << _side << std::endl;
                std::clog << "<log> data: " << data << std::endl;

                if(_data[i].getPtr() == _side){
                    _side = (Unit *)data;

                    return;
                }

                if(data == _side){
                    unsigned rightMostOffset = getRightMostOffset();
                    _data[rightMostOffset].addKey(idx);

                    return;
                }

                if(_data[i].getBitmap(0) && _data[i].getPtr() == data){
                    _data[i].addKey(idx);

                    return;
                }
            }

            KeyPtrSet newData(_options.kp_length, false);
            newData.setPtr(data);
            newData.addKey(idx);

            bool insertSide = false;
            unsigned shiftPos = getShiftPosition();
            unsigned insertPos = getInsertPosition(idx, insertSide);

            //TODO
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
            //std::clog << "<log> insertPos: " << insertPos << std::endl;
            //std::clog << "<log> insertSide: " << insertSide << std::endl;

            if(_side != nullptr){
                if(insertSide){
                    void *temp = _side;
                    _side = (Unit *)_data[insertPos].getPtr();
                    _data[insertPos].setPtr(temp);
                }
                else if(!insertSide && split){
                    int i = 0;
                    while(!_data[insertPos+i+1].getBitmap(0))++i; //TODO
                    void *temp = _data[insertPos+i+1].getPtr();
                    _data[insertPos+i+1].setPtr(_data[insertPos].getPtr());
                    _data[insertPos].setPtr(temp);
                }
            }
        }
    }

    /**
     * * Find and delete index 
     * TODO evaluation 
     * TODO general unit parameter 
     * TODO redesign as deleteData(offset, side) to direct delete (still need to check right most) 
     * @param side for internal, if true, and try to delete the right most index, that will delete the side unit
     */
    void deleteData(unsigned idx, bool side = false){
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

        for(int i = 0; i < _options.track_length; ++i){
            if(_data[i].getBitmap(0) && idx == _data[i].getKey(0)){ //TODO
                // delete side unit
                if(!_isLeaf && isRightMostOffset(i) && side){
                    connectSideUnit((Unit *)_data[i].getPtr());
                }
                deleteMark(i);
                return;
            }
        }

        throw "delete not found";
    }

    /* Minor Functions */

    void connectSideUnit(Unit *unit){
        _side = unit;
        _sideBitmap = true;
    }

    void connectParentNode(Unit *unit){
        _parent = unit;
    }

    void deleteMark(unsigned offset, bool side = false){
        if(_isLeaf){
            _data[offset].delKey(0);
        }
        else{
            _data[offset / _options.unit_size].delKey(offset % _options.unit_size);
        }
    }

    unsigned getOffsetByIndex(unsigned idx){
        for(int i = 0; i < _options.track_length; ++i){
            for(int j = 0; j < _options.unit_size; ++j){
                if(_data[i].getBitmap(j) && idx == _data[i].getKey(j)){
                    return i + j;
                }
            }
        }

        return _options.track_length * _options.unit_size;
    }

    unsigned getMinIndex(){
        for(int i = 0; i < _options.track_length; ++i){
            for(int j = 0; j < _options.unit_size; ++j){
                if(_data[i].getBitmap(j)){
                    return _data[i].getKey(j);
                }
            }
        }
    }

    unsigned getMaxIndex(){
        for(int i = _options.track_length - 1; i >= 0; --i){
            for(int j = _options.unit_size - 1; j >= 0; --j){
                if(_data[i].getBitmap(j)){
                    return _data[i].getKey(j);
                }
            }
        }
    }

    KeyPtrSet getMinData(){
        for(int i = 0; i < _options.track_length; ++i){
            for(int j = 0; j < _options.unit_size; ++j){
                if(_data[i].getBitmap(j)){
                    return _data[i]; //?
                }
            }
        }
    }

    KeyPtrSet getMaxData(){
        for(int i = _options.track_length - 1; i >= 0; --i){
            for(int j = _options.unit_size - 1; j >= 0; --j){
                if(_data[i].getBitmap(j)){
                    return _data[i];
                }
            }
        }
    }

    unsigned getLeftMostOffset(){
        if(_isLeaf){
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0))
                    return i;
            }
        }
        else{
            //TODO
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0))
                    return i;
            }
        }

        return -1;
    }

    unsigned getRightMostOffset(){
        if(_isLeaf){
            for(int i = _options.track_length - 1; i >= 0; --i){
                if(_data[i].getBitmap(0))
                    return i;
            }
        }
        else{
            //TODO
            for(int i = _options.track_length - 1; i >= 0; --i){
                if(_data[i].getBitmap(0))
                    return i;
            }
        }

        return -1;
    }

    bool isRightMostOffset(unsigned offset) const{
        if(offset == _options.track_length)
            return true;
            
        if(offset == -1)
            return false;

        if(!_data[offset / _options.unit_size].getBitmap(offset % _options.unit_size))
            return false;

        for(int i = offset + 1; i < _options.track_length; ++i){
            if(_data[i].getBitmap(0)) //TODO
                return false;
        }

        return true;
    }

    bool isLeftMostOffset(unsigned offset) const{
        if(offset == -1 || offset == _options.track_length)
            return false;
        
        if(!_data[offset / _options.unit_size].getBitmap(offset % _options.unit_size))
            return false;

        for(int i = 0; i < offset; ++i){
            if(_data[i].getBitmap(0)) //TODO
                return false;
        }
        //std::clog << "<log> offset: " << offset << std::endl;
        //std::clog << "<log> _options.unit_size: " << _options.unit_size << std::endl;

        return true;
    }

    unsigned getClosestRightOffset(unsigned offset){
        for(int i = offset + 1; i < _options.track_length; ++i){
            if(_data[i].getBitmap(0)){ //TODO
                return i;
            }
        }

        throw "getCloseRightIndex() fail";
    }

    unsigned getClosestLeftOffset(unsigned offset){
        for(int i = offset - 1; i >= 0; --i){
            if(_data[i].getBitmap(0)){ //TODO
                return i;
            }
        }

        throw "getCloseLeftIndex() fail";
    }
    
    // Return insert position
    unsigned getInsertPosition(unsigned wait_insert_idx, bool &insertSide){
        if(_isLeaf){
            switch (_options.insert_mode){
            case Options::insert_function::SEQUENTIAL:
                switch (_options.node_ordering){
                case Options::ordering::SORTED:
                {
                    unsigned last = 0;
                    for(int i = 0; i < _options.track_length; ++i){
                        if(_data[i].getBitmap(0)){ //TODO
                            last = i + 1;
                        }
                    }
                   
                   //TODO
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
                case Options::ordering::UNSORTED:
                    for(int i = 0; i < _options.track_length; ++i){
                        if(!_data[i].getBitmap(0)){ //TODO
                            return i;
                        }
                    }
                    throw "full";
                default:
                    throw "undefined ordering";
                }
                break;
            case Options::insert_function::BIT_BINARY_INSERT:
                /* code */
                throw "Developing";
            default:
                throw "undefined insert operation";
            }
        }
        else{
            unsigned last = 0;
            for(int i = 0; i < _options.track_length; ++i){
                if(_data[i].getBitmap(0)){ //TODO
                    last = i + 1;
                }
            }
            
            //TODO
            for(int i = 0; i < last; ++i){
                if(!_data[i].getBitmap(0) && _data[i+1].getBitmap(0) && wait_insert_idx < _data[i+1].getKey(i % _options.unit_size)){
                    return i;
                }
                else if(_data[i].getBitmap(0) && wait_insert_idx < _data[i].getKey(i % _options.unit_size)){
                    return i;
                }
            }

            insertSide = true;
            return last == _options.track_length ? last - 1 : last;
        }
        
    }

    unsigned getShiftPosition(){
        unsigned shiftPoint = -1;
        for(int i = 0; i < _options.track_length; ++i){
            if(!_data[i].getBitmap(0)){ //TODO
                shiftPoint = i;
                break;
            }
        }
        if(shiftPoint == -1)
            throw "full";
        
        return shiftPoint;
    }

    bool isHalf() const{
        unsigned counter = 0;
        for(int i = 0; i < _options.track_length; ++i){
            if(_data[i].getBitmap(0)){ //TODO
                counter++;
            }
        }

        return counter == _options.track_length / 2;
    }

    unsigned getSize(){
        unsigned counter = 0;
        for(int i = 0; i < _options.track_length; ++i){
            if(_data[i].getBitmap(0)){ //TODO
                counter++;
            }
        }

        return counter;
    }

    /* Data Member (Data) */
    KeyPtrSet *_data;

    /* Data Member (Meta Data) */
    //? bool *_bitmap;
    bool _isLeaf;
    Unit *_side;
    bool _sideBitmap;
    Unit *_parent;
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
    out << "\t\t(\n";

    //* status
    out << "\t\t\t" << right._shiftCounter << "\n";

    out << "\t\t\t";
    bool first = true;
    for(int i = 0; i < right._options.track_length; ++i){
        if(first)first = false;
        else out << ", ";
        out << right._data[i];
    }
    out << " _side: " << right._side;

    out << "\n\t\t)";

    return out;
}

#endif