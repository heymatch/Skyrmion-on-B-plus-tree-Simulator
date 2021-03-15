/**
 *
 *
 **/

#ifndef NODE_H
#define NODE_H

#include "Counter.hpp"
#include "Options.hpp"
#include "Unit.hpp"
#include <ostream>

struct Unit;

struct Node{
    Node(Options options = Options(), bool None = true, bool isLeaf = true) : _options(options){
        if(!None){
            _data = new KeyPtrSet[_options.track_length]();
            _bitmap = new bool[_options.track_length]();
            _isLeaf = isLeaf;
            _side = nullptr;
            _parent = nullptr;
            _id = NodeId++;
            _sideBitmap = false;
            if(!isLeaf){
                for(int i = 0; i < _options.track_length; ++i){
                    _data[i] = KeyPtrSet(_options.kp_length);
                }
            }
        }
    }

    // Return array of KeyPtrSet
    // 0 index
    KeyPtrSet *readData(unsigned lower, unsigned upper){
        if(upper < lower)
            throw "invalid read bound";
         
        // calculate performance
        switch (_options.read_mode)
        {
        case Options::read_function::SEQUENTIAL:
            /* code */
            break;
        case Options::read_function::RANGE_READ:
            /* code */
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

    // Return data pointer
    void *searchData(unsigned idx){
        if(_isLeaf){
            switch (_options.search_mode)
            {
            case Options::search_function::SEQUENTIAL:
                for(int i = 0; i < _options.track_length; ++i){
                    if(_bitmap[i] && _data[i].getKey(0) == idx){
                        return _data[i].getPtr();
                    }
                }
                throw -1;
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
            for(int i = 0; i < _options.track_length; ++i){
                if(_bitmap[i] && idx < _data[i].getKey(0)){
                    return _data[i].getPtr();
                }
            }
            return _side;
        }
    }
    

    void updateData(unsigned idx, unsigned data){
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

    void updateIndex(unsigned offset, unsigned idx){
        _data[offset].setKey(0, idx);
    }

    // Insert KeyPtrSet
    void insertData(unsigned idx, void *data, bool split = true){
        // read node(metadata, bitmap and data)
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
                    if(_bitmap[insertPos]){
                        if(shiftPos < insertPos){
                            if(!insertSide)
                                insertPos -= 1;
                            for(int i = shiftPos; i < insertPos; ++i){
                                _data[i] = _data[i+1];
                                _bitmap[i] = _bitmap[i+1];
                            }
                        }
                        else{
                            for(int i = shiftPos; i > insertPos; --i){
                                _data[i] = _data[i-1];
                                _bitmap[i] = _bitmap[i-1];
                            }
                        }
                    }
                    //std::clog << "<log> insertPos: " << insertPos << std::endl;
                    //std::clog << "<log> _data[insertPos]: " << _data[insertPos] << std::endl;
                    //std::clog << "<log> idx: " << idx << std::endl;
                    _bitmap[insertPos] = true;
                    _data[insertPos] = newData;
                }
                    break;
                case Options::ordering::UNSORTED:
                    _data[insertPos] = newData;
                    _bitmap[insertPos] = true;
                    break;
                default:
                    throw "undefined insert operation";
            }
        }
        else{
            KeyPtrSet newData(_options.kp_length, false);
            newData.setPtr(data);
            newData.addKey(idx);

            bool insertSide = false;
            unsigned shiftPos = getShiftPosition();
            unsigned insertPos = getInsertPosition(idx, insertSide);

            //std::clog << "<log> insertPos: " << insertPos << std::endl;
            //std::clog << "<log> shiftPos: " << shiftPos << std::endl;
            //std::clog << "<log> idx: " << idx << std::endl;

            if(_bitmap[insertPos]){
                if(shiftPos < insertPos){
                    if(!insertSide)
                        insertPos -= 1;
                    for(int i = shiftPos; i < insertPos; ++i){
                        _data[i] = _data[i+1];
                        _bitmap[i] = _bitmap[i+1];
                    }
                }
                else{
                    for(int i = shiftPos; i > insertPos; --i){
                        _data[i] = _data[i-1];
                        _bitmap[i] = _bitmap[i-1];
                    }
                }
            }

            _bitmap[insertPos] = true;
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
                    while(!_bitmap[insertPos+i+1])++i;
                    void *temp = _data[insertPos+i+1].getPtr();
                    _data[insertPos+i+1].setPtr(_data[insertPos].getPtr());
                    _data[insertPos].setPtr(temp);
                }
            }
        }
    }

    void connectSideUnit(Unit *unit){
        _side = unit;
        _sideBitmap = true;
    }

    void connectParentNode(Unit *unit){
        _parent = unit;
    }

    void deleteData(unsigned idx, bool side = false){
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
        }

        for(int i = 0; i < _options.track_length; ++i){
            if(_bitmap[i] && idx == _data[i].getKey(0)){
                // delete side unit
                if(!_isLeaf && isRightMostOffset(i) && side){
                    //std::clog << "<log> delete side" << std::endl;
                    connectSideUnit((Unit *)_data[i].getPtr());
                }
                deleteMark(i);
                return;
            }
        }

        throw "delete not found";
    }

    void deleteMark(unsigned *arr, unsigned arrSize){
        for(int i = 0; i < arrSize; ++i){
            _bitmap[arr[i]] = false;
        }
    }

    void deleteMark(unsigned offset, bool side = false){
        _bitmap[offset] = false;
    }

    unsigned getOffsetByIndex(unsigned idx){
        for(int i = 0; i < _options.track_length; ++i){
            if(_bitmap[i] && idx == _data[i].getKey(0)){
                return i;
            }
        }
        return _options.track_length;
    }

    unsigned getMinIndex(){
        for(int i = 0; i < _options.track_length; ++i){
            if(_bitmap[i]){
                return _data[i].getKey(0);
            }
        }
    }

    KeyPtrSet getMinData(){
        for(int i = 0; i < _options.track_length; ++i){
            if(_bitmap[i]){
                return _data[i];
            }
        }
    }

    KeyPtrSet getMaxData(){
        for(int i = _options.track_length-1; i >= 0; --i){
            if(_bitmap[i]){
                return _data[i];
            }
        }
    }

    unsigned getMaxIndex(){
        for(int i = _options.track_length-1; i >= 0; --i){
            if(_bitmap[i]){
                return _data[i].getKey(0);
            }
        }
    }

    unsigned getLeftMostOffset(){
        for(int i = 0; i < _options.track_length; ++i){
            if(_bitmap[i])
                return i;
        }

        return -1;
    }

    unsigned getRightMostOffset(){
        for(int i = _options.track_length - 1; i >= 0; --i){
            if(_bitmap[i])
                return i;
        }

        return -1;
    }

    bool isRightMostOffset(unsigned offset) const{
        if(offset == -1)
            return false;

        if(!_bitmap[offset])
            return false;

        for(int i = offset + 1; i < _options.track_length; ++i){
            if(_bitmap[i])
                return false;
        }

        return true;
    }

    bool isLeftMostOffset(unsigned offset) const{
        if(offset == -1)
            return false;
        //std::clog << "<log> offset: " << offset << std::endl;
        if(!_bitmap[offset])
            return false;

        for(int i = 0; i < offset; ++i){
            if(_bitmap[i])
                return false;
        }

        return true;
    }

    unsigned getClosestRightOffset(unsigned offset){
        for(int i = offset + 1; i < _options.track_length; ++i){
            if(_bitmap[i]){
                return i;
            }
        }

        throw "getCloseRightIndex() fail";
    }

    unsigned getClosestLeftOffset(unsigned offset){
        for(int i = offset - 1; i >= 0; --i){
            if(_bitmap[i]){
                return i;
            }
        }

        throw "getCloseLeftIndex() fail";
    }
    
    // Return insert position
    unsigned getInsertPosition(unsigned wait_insert_idx, bool &insertSide){
        if(_isLeaf){ // Check Leaf or Internal
            switch (_options.insert_mode){
            case Options::insert_function::SEQUENTIAL:
                switch (_options.node_ordering){
                case Options::ordering::SORTED:
                    /* EVALUATE READ AND SHIFT */
                {
                    unsigned last = 0;
                    for(int i = 0; i < _options.track_length; ++i){
                        if(_bitmap[i]){
                            last = i + 1;
                        }
                    }
                   
                    for(int i = 0; i < last; ++i){
                        if(!_bitmap[i] && _bitmap[i+1] && wait_insert_idx < _data[i+1].getKey(0)){
                            return i;
                        }
                        else if(_bitmap[i] && wait_insert_idx < _data[i].getKey(0)){
                            return i;
                        }
                    }
                    
                    insertSide = true;
                    return last == _options.track_length ? last - 1 : last;
                }
                case Options::ordering::UNSORTED:
                    /* EVALUATE READ AND SHIFT */
                    for(int i = 0; i < _options.track_length; ++i){
                        if(!_bitmap[i]){
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
            switch (_options.insert_mode)
            {
            case Options::insert_function::SEQUENTIAL:
                /* EVALUATE READ AND SHIFT */
                break;
            case Options::insert_function::BIT_BINARY_INSERT:
                /* EVALUATE READ AND SHIFT */
                break;
            default:
                throw "undefined insert operation";
            }

            unsigned last = 0;
            for(int i = 0; i < _options.track_length; ++i){
                if(_bitmap[i]){
                    last = i + 1;
                }
            }
            
            for(int i = 0; i < last; ++i){
                if(!_bitmap[i] && _bitmap[i+1] && wait_insert_idx < _data[i+1].getKey(0)){
                    return i;
                }
                else if(_bitmap[i] && wait_insert_idx < _data[i].getKey(0)){
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
            if(!_bitmap[i]){
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
            if(_bitmap[i]){
                counter++;
            }
        }

        return counter == _options.track_length / 2;
    }

    KeyPtrSet *_data;
    //unsigned *_index;
    //void **_ptr;

    // Metadata
    bool *_bitmap;
    bool _isLeaf;
    Unit *_side;
    bool _sideBitmap;
    Unit *_parent;
    //bool _isValid;
    ////
    Options _options;
    unsigned _id;
};

std::ostream &operator<<(std::ostream &out, const Node &right){
    //std::clog << "<log> Node Print" << std::endl;
    /// status
    out << "(";
    out << "Node";
    if(right._isLeaf) out << " Leaf";
    else out << " Internal";
    out << " " << right._id;
    out << " " << &right;
    out << "\t";
    ///
    bool first = true;
    for(int i = 0; i < right._options.track_length; ++i){
        if(first)first = false;
        else out << ", ";
        out << right._data[i];
        if(!right._bitmap[i]) out << "*";
    }
    out << " _side: " << right._side;
    out << ")";
    return out;
}

#endif