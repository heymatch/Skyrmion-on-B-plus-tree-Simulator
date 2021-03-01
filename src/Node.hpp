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
    

    void updateData(unsigned index, unsigned data){
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

    // Insert KeyPtrSet
    void insertData(unsigned idx, void *data){
        // read node(metadata, bitmap and data)
        if(_isLeaf){
            KeyPtrSet newData(2);
            newData.setPtr(data);
            newData.addKey(idx);

            bool insertSide = true;
            unsigned insertPos = getInsertPosition(idx, insertSide);

            switch (_options.node_ordering){
                case Options::ordering::SORTED:
                {
                    bool shift = false;
                    for(int i = _options.track_length-1; i > insertPos; --i){
                        if(!shift && !_bitmap[i]){
                            shift = true;
                        }
                        if(shift){
                            _data[i] = _data[i-1];
                            _bitmap[i] = _bitmap[i-1];
                        }
                    }
                    //std::clog << "<log> insertPos: " << insertPos << std::endl;
                    //std::clog << "<log> _data[insertPos]: " << _data[insertPos] << std::endl;
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
            unsigned insertPos = getInsertPosition(idx, insertSide);

            bool shift = false;
            for(int i = _options.track_length-1; i > insertPos; --i){
                if(!shift && !_bitmap[i]){
                    shift = true;
                }
                if(shift){
                    _data[i] = _data[i-1];
                    _bitmap[i] = _bitmap[i-1];
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
                else{
                    void *temp = _data[insertPos+1].getPtr();
                    _data[insertPos+1].setPtr(_data[insertPos].getPtr());
                    _data[insertPos].setPtr(temp);
                }
            }
        }
    }
/*
    // For internal node, ptr points to next unit
    void connectUnit(unsigned idx, Unit *unit){
        // read node(metadata, bitmap and data) 
        bool insertSide = false;
        unsigned insertPos = getInsertPosition(idx, insertSide); // EVALUATE SEARCH 
        //std::clog << "<log> idx: " << idx << std::endl;
        //std::clog << "<log> insertPos: " << std::boolalpha << insertSide << std::endl;
        if(insertSide && _side != nullptr){
            _bitmap[insertPos] = true;
            _index[insertPos] = idx;
            _ptr[insertPos] = _side;
            connectSideUnit(unit);
        }
        else{
            bool shift = false;
            for(int i = _options.track_length-1; i > insertPos; --i){
                if(!shift && !_bitmap[i]){
                    shift = true;
                }
                if(shift){
                    _index[i] = _index[i-1];
                    _bitmap[i] = _bitmap[i-1];
                    if(i != insertPos+1){
                        _ptr[i] = _ptr[i-1];
                    }
                }
            }
            
            _bitmap[insertPos] = true;
            _index[insertPos] = idx;
            // EVALUATE INSERTION 
            if(_ptr[insertPos] == nullptr)
                _ptr[insertPos] = unit;
            else 
                _ptr[insertPos+1] = unit;
            
            
            // std::clog << unit << std::endl;
            // std::clog << _ptr[insertPos] << std::endl;
            // for(int i = 0; i < _options.track_length; ++i){
            //     std::clog << _ptr[i] << " "; 
            // }
            // std::clog << std::endl;
            
        }
        
    }*/

    void connectSideUnit(Unit *unit){
        _side = unit;
        _sideBitmap = true;
    }

    void connectParentNode(Unit *unit){
        _parent = unit;
    }

    void deleteData(unsigned data){
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
    }

    void deleteMark(unsigned *arr, unsigned arrSize){
        for(int i = 0; i < arrSize; ++i){
            _bitmap[arr[i]] = false;
        }
    }

    void deleteMark(unsigned idx){
        _bitmap[idx] = false;
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
                    bool full = true;
                    unsigned last = 0;
                    for(int i = 0; i < _options.track_length; ++i){
                        if(!_bitmap[i]){
                            full = false;
                        }
                        else{
                            last = i + 1;
                        }
                    }
                    if(full) throw "full";
                   
                    for(int i = 0; i < last; ++i){
                        if(_bitmap[i] && wait_insert_idx < _data[i].getKey(0)){
                            return i;
                        }
                    }
                    
                    
                    return last;
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

            bool full = true;
            unsigned last = 0;
            for(int i = 0; i < _options.track_length; ++i){
                if(!_bitmap[i]){
                    full = false;
                }
                else{
                    last = i + 1;
                }
            }
            if(full) throw "full";
            
            for(int i = 0; i < last; ++i){
                if(_bitmap[i] && wait_insert_idx < _data[i].getKey(0)){
                    return i;
                }
            }

            insertSide = true;
            return last;
        }
        
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
    out << "(";
    bool first = true;
    for(int i = 0; i < right._options.track_length; ++i){
        if(first)first = false;
        else out << ", ";
        out << right._data[i];
        if(!right._bitmap[i]) out << "*";
    }
    out << ")";
    return out;
}

#endif