#ifndef UNIT_H
#define UNIT_H

#include <ostream>

#include "Options.hpp"
#include "KeyPtrSet.hpp"
#include "Counter.hpp"

namespace Evaluation{
    Size countSkyrmion(Index data) {
        Size number = 0;
        Size a = 0;
        while(data){
            if(data % 2) {
                number++;
            }
            data >>= 1;
        }
        return number;
    }

    Size countSkyrmion(KeyPtrSet data) {
        Size number = 0;
        number += countSkyrmion((Index)data.getPtr());
        for(int i = 0; i < data.getKeyCapacity(); ++i){
            number += countSkyrmion(data.getKey(i));
        }
        return number;
    }

    Size log2(Size number) {
        Size counter = 0;
        Size ceil = countSkyrmion(number) != 1;
        while(number >>= 1){
            counter++;
        }
        return counter + ceil;
    }

    void sequential_read(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter){
        shiftCounter.count(2 * options.track_length);  
        readCounter.count(options.track_length);   
    }

    void sequential_read_half(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter){
        shiftCounter.count(options.track_length);  
        readCounter.count(options.track_length / 2);   
    }

    void range_read(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter){
        shiftCounter.count(2 * options.word_length);
        readCounter.count(options.track_length);
    }

    void range_read_half(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter){
        shiftCounter.count(2 * options.word_length);
        readCounter.count(options.track_length / 2);
    }

    void overwrite(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter, Counter &insertCounter, Counter &removeCounter, const KeyPtrSet &oldData, const KeyPtrSet &newData){
        //* remove old data
        if(isLeaf){
            shiftCounter.count(options.word_length * 2);
            removeCounter.count(options.word_length * 2);
        }
        else{
            shiftCounter.count(options.word_length * options.kp_length);
            removeCounter.count(options.word_length * options.kp_length);
        }
        
        //* write new data
        insertCounter.count(countSkyrmion( newData ));
        if(isLeaf){
            shiftCounter.count(options.word_length * 2);
        }
        else{
            shiftCounter.count(options.word_length * options.kp_length);
        }
    }

    void overwrite(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter, Counter &insertCounter, Counter &removeCounter, const Index &oldIndex, const Index &newIndex){
        //* remove old data
        shiftCounter.count(options.word_length);
        removeCounter.count(options.word_length);

        //* write new data
        insertCounter.count(countSkyrmion( newIndex ));
        shiftCounter.count(options.word_length);
    }

    void permutation_write(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter, Counter &insertCounter, Counter &removeCounter, const KeyPtrSet &oldData, const KeyPtrSet &newData){
        //* read old data
        if(isLeaf){
            shiftCounter.count(options.word_length * 2);
            readCounter.count(options.word_length * 2);
        }
        else{
            shiftCounter.count(options.word_length * options.kp_length);
            readCounter.count(options.word_length * options.kp_length);
        }

        //* write new data
        Size oldDataSkyrmion = countSkyrmion( oldData );
        Size newDataSkyrmion = countSkyrmion( newData );
        if(oldDataSkyrmion < newDataSkyrmion){
            insertCounter.count(newDataSkyrmion - oldDataSkyrmion);
        }
        else{
            removeCounter.count(oldDataSkyrmion - newDataSkyrmion);
        }

        if(isLeaf){
            shiftCounter.count(options.word_length * 2);
        }
        else{
            shiftCounter.count(options.word_length * options.kp_length);
        }
    }

    void permutation_write(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter, Counter &insertCounter, Counter &removeCounter, const Index &oldIndex, const Index &newIndex){
        //* read old data
        shiftCounter.count(options.word_length);
        readCounter.count(options.word_length);

        //* write new data
        Size oldDataSkyrmion = countSkyrmion( oldIndex );
        Size newDataSkyrmion = countSkyrmion( newIndex );
        if(oldDataSkyrmion < newDataSkyrmion){
            insertCounter.count(newDataSkyrmion - oldDataSkyrmion);
        }
        else{
            removeCounter.count(oldDataSkyrmion - newDataSkyrmion);
        }

        shiftCounter.count(options.word_length);
    }

    void permutation_write_node(){
        
    }

    void permute(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter, const KeyPtrSet oldData[], const KeyPtrSet newData[]){
        if(options.read_mode == Options::read_function::SEQUENTIAL){
            sequential_read(options, isLeaf, readCounter, shiftCounter);
        }
        else if(options.read_mode == Options::read_function::RANGE_READ){
            range_read(options, isLeaf, readCounter, shiftCounter);
        }

        // int oldBit[options.track_length] = {};
        // int newBit[options.track_length] = {};
        
        // int b = options.track_length - 1;
        // for(int i = options.dataSize(isLeaf) - 1; i >= 0; --i){
        //     Size v = (Size) oldData[i].getPtr();
        //     for(int j = options.word_length - 1; j >= 0; --j){
        //         oldBit[b--] = v & 1;
        //         v >>= 1;
        //     }

        //     for(int k = 0; k < oldData[i].getKeyCapacity(); ++k){
        //         v = (Size) oldData[i].getKey(k);
        //         for(int j = options.word_length - 1; j >= 0; --j){
        //             oldBit[b--] = v & 1;
        //             v >>= 1;
        //         }
        //     }
        // }

        // b = options.track_length - 1;
        // for(int i = options.dataSize(isLeaf) - 1; i >= 0; --i){
        //     Size v = (Size) newData[i].getPtr();
        //     for(int j = options.word_length - 1; j >= 0; --j){
        //         newBit[b--] = v & 1;
        //         v >>= 1;
        //     }

        //     for(int k = 0; k < newData[i].getKeyCapacity(); ++k){
        //         v = (Size) newData[i].getKey(k);
        //         for(int j = options.word_length - 1; j >= 0; --j){
        //             newBit[b--] = v & 1;
        //             v >>= 1;
        //         }
        //     }
        // }

        // Size permuteRange = 0;
        // Size write_1 = 0;
        // Size write_0 = 0;

        // //* spacing shift
        // for(int i = 0; i < options.track_length; ++i){
        //     if(oldBit[i] != newBit[i]){
        //         permuteRange = options.track_length - i;
        //         break;
        //     }
        // }
        // shiftCounter.count(permuteRange);

        // //* write 1
        // for(int i = 0, j = 0; i < options.track_length; ++i){
        //     if(newBit[i] == 1){
        //         while(oldBit[j] == 0){
        //             ++write_1;
        //             ++j;
        //         }
        //         ++j;
        //     }
        // }
        // shiftCounter.count(write_1);

        // //* write 0
        // Size old_b = 0, new_b = 0;
        // bool old_start = false, new_start = false;

        // for(int i = 0, j = 0; i < options.track_length; ++i){
        //     if(old_start && oldBit[i] == 0){
        //         ++old_b;
        //     }
        //     else if(oldBit[i] == 1){
        //         old_start = true;
        //     }
        // }

        // for(int i = 0, j = 0; i < options.track_length; ++i){
        //     if(new_start && newBit[i] == 0){
        //         ++new_b;
        //     }
        //     else if(newBit[i] == 1){
        //         new_start = true;
        //     }
        // }

        // write_0 = old_b < new_b ? new_b - old_b : old_b - new_b;
        // shiftCounter.count(write_0);
    }

    void insert(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter, Counter &insertCounter, const KeyPtrSet &newData){
        //* write new data
        if(isLeaf){
            shiftCounter.count(options.word_length * 2);
        }
        else{
            shiftCounter.count(options.word_length * options.kp_length);
        }
        insertCounter.count(countSkyrmion( newData ));
    }

    void binary_search(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter, KeyPtrSet data[], const Index &searchKey){
        if(isLeaf){
            int l = 0;
            int h = 0;
            for(int i = options.dataSize(isLeaf) - 1; i >= 0; --i){
                if(data[i].getBitmap(0)){
                    h = i;
                    break;
                }
            }

            bool left = true;
            while (l <= h) {
                int mid = (l + h) / 2;
                while(!data[mid].getBitmap(0)){
                    if(mid > l){
                        --mid;
                    }
                    else{
                        break;
                    }
                }
                while(!data[mid].getBitmap(0)){
                    if(mid < h){
                        ++mid;
                    }
                    else{
                        break;
                    }
                }

                shiftCounter.count(2 * options.word_length);
                readCounter.count(options.word_length);
                if(data[mid].getKey(0) > searchKey) {
                    h = mid - 1;
                }
                else if(data[mid].getKey(0) < searchKey) {
                    l = mid + 1;
                }
                else{
                    break;
                }
            }
        }
        else{
            shiftCounter.count(2 * options.word_length * log2(options.dataSize(isLeaf) * options.unit_size));
            readCounter.count(options.word_length * log2(options.dataSize(isLeaf) * options.unit_size));
        }
    }

    void bit_binary_search(const Options &options, const bool &isLeaf, Counter &readCounter, Counter &shiftCounter, KeyPtrSet data[], const Index &searchKey){
        if(isLeaf){
            shiftCounter.count(2 * options.word_length);
            readCounter.count(options.track_length);
        }
        else{
            shiftCounter.count(2 * options.word_length);
            readCounter.count(options.track_length);
        }
    }

    void migrate(const Options &options, const bool &isLeaf, Counter &shiftCounter, Counter &migrateCounter, KeyPtrSet *Data){
        if(isLeaf){

        }
        else{
            
        }
    }

};

struct Unit;

namespace System{
    Unit *allocUnit(Options options, bool isLeaf = true);
}

struct Unit{
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

                _data = new KeyPtrSet[_options.dataSize(_isLeaf)]();
                if(!isLeaf){
                    for(int i = 0; i < _options.dataSize(_isLeaf); ++i){
                        _data[i] = KeyPtrSet(_options.kp_length);
                    }
                }

                _id = NodeId++;
            }
        }

        Node(const Node &right){
            _options = right._options;

            _isLeaf = right._isLeaf;
            _sideBack = right._sideBack;
            _sideBackBitmap = right._sideBackBitmap;
            _sideFront = right._sideFront;
            _sideFrontBitmap = right._sideFrontBitmap;
            _parent = right._parent;
            _parentOffset = right._parentOffset;
            _isValid = right._isValid;

            _id = right._id;

            _data = new KeyPtrSet[_options.dataSize(_isLeaf)]();
            for(int i = 0; i < _options.dataSize(_isLeaf); ++i){
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

            _isLeaf = right._isLeaf;
            _sideBack = right._sideBack;
            _sideBackBitmap = right._sideBackBitmap;
            _sideFront = right._sideFront;
            _sideFrontBitmap = right._sideFrontBitmap;
            _parent = right._parent;
            _parentOffset = right._parentOffset;
            _id = right._id;
            _isValid = right._isValid;

            _data = new KeyPtrSet[_options.dataSize(_isLeaf)]();

            //_shiftCounter = right._shiftCounter;
            for(int i = 0; i < _options.dataSize(_isLeaf); ++i){
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
        void *searchData(const Index &idx, Offset &next_unit_offset){
            switch (_options.search_mode){
                case Options::search_function::SEQUENTIAL:
                    //TODO evaluation 
                    break;
                case Options::search_function::TRAD_BINARY_SEARCH:
                    Evaluation::binary_search(_options, isLeaf(), _readCounter, _shiftCounter, _data, idx);
                    break;
                case Options::search_function::BIT_BINARY_SEARCH:
                    Evaluation::bit_binary_search(_options, isLeaf(), _readCounter, _shiftCounter, _data, idx);
                    break;
                default:
                    throw "undefined search operation";
            }

            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                if(_isLeaf){
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                        if(_data[i].getBitmap(0) && _data[i].getKey(0) == idx){
                            return _data[i].getPtr();
                        }
                    }

                    return nullptr;
                }
                else{
                    for(int i = _options.dataSize(isLeaf()) - 1; i >= 0 ; --i){
                        if(_data[i].getBitmap(0) && idx >= _data[i].getKey(0)){
                            return _data[i].getPtr();
                        }
                    }
                    
                    return _sideFront;
                }
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                if(_isLeaf){
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                        if(_data[i].getBitmap(0) && _data[i].getKey(0) == idx){
                            return _data[i].getPtr();
                        }
                    }
                    return nullptr;
                }
                else{
                    for(int i = _options.dataSize(isLeaf()) - 1; i >= 0; --i){
                        for(int j = _options.unit_size - 1; j >= 0; --j){
                            if(_data[i].getBitmap(j) && idx >= _data[i].getKey(j)){
                                next_unit_offset = j;
                                return _data[i].getPtr();
                            }
                        } 
                    }

                    next_unit_offset = _options.unit_size;
                    return _sideFront;      
                }
            }
            
            throw "<Node> search error";
        }
        
        //?
        void updateData(Index idx, Data data){
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
        void insertData(const Index &idx, void *data, Offset &insertPosition, bool directInsert = false, Size migrateSkyrmion = 0){
            
            //* find the insert position
            if(!directInsert){
                switch (_options.search_mode){
                    case Options::search_function::SEQUENTIAL:
                        //TODO evaluation 
                        break;
                    case Options::search_function::TRAD_BINARY_SEARCH:
                        Evaluation::binary_search(_options, isLeaf(), _readCounter, _shiftCounter, _data, idx);
                        break;
                    case Options::search_function::BIT_BINARY_SEARCH:
                        Evaluation::bit_binary_search(_options, isLeaf(), _readCounter, _shiftCounter, _data, idx);
                        break;
                    default:
                        throw "undefined search operation";
                }
            }

            if(_isLeaf){
                KeyPtrSet newData(2, true);
                newData.setPtr(data);
                newData.addKey(idx);

                Offset shiftPos = getShiftPosition();
                insertPosition = getInsertPosition(idx, shiftPos);

                Size overwrittenSkyrmion = Evaluation::countSkyrmion(_data[shiftPos]);
                Node oldData = *this;

                if(_options.node_ordering == Options::ordering::SORTED){
                    if(_data[insertPosition].getBitmap(0)){
                        if(shiftPos < insertPosition){
                            for(int i = shiftPos; i < insertPosition; ++i){
                                _data[i] = _data[i+1];
                            }
                        }
                        else{
                            for(int i = shiftPos; i > insertPosition; --i){
                                _data[i] = _data[i-1];
                            }
                        }

                        //* Evaluate for insert a data
                        Evaluation::insert(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, newData);
                    }

                    _data[insertPosition] = newData;

                    if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                        int64_t diff = diffSkyrmion(oldData._data);
                        if(diff > 0){
                            if(migrateSkyrmion > 0){
                                _migrateCounter.count(migrateSkyrmion);
                                diff -= migrateSkyrmion;
                                diff = diff < 0 ? 0 : diff;
                            }
                            _insertCounter.count(diff);
                            insertSkyrmion(diff + overwrittenSkyrmion);
                        }
                        else{
                            _removeCounter.count(-diff);
                            removeSkyrmion(-diff);
                        }
                    }

                    switch (_options.update_mode){
                    case Options::update_function::OVERWRITE:
                        Evaluation::overwrite(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, _removeCounter, _data[insertPosition], newData);
                        break;
                    case Options::update_function::PERMUTATION_WRITE:
                        Evaluation::permutation_write(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, _removeCounter, _data[insertPosition], newData);
                        break;
                    case Options::update_function::PERMUTE_WORD_COUNTER:
                        //TODO evaluation 
                        break;
                    case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                        Evaluation::permute(_options, isLeaf(), _readCounter, _shiftCounter, oldData._data, _data);
                        break;
                    default:
                        throw "undefined update operation";
                        break;
                    }

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
                    KeyPtrSet newData(_options.kp_length, false);
                    newData.setPtr(data);
                    newData.addKey(idx);

                    Offset shiftPos = getShiftPosition();
                    insertPosition = getInsertPosition(idx, shiftPos);

                    Size overwrittenSkyrmion = Evaluation::countSkyrmion(_data[shiftPos]);
                    Node oldData = *this;

                    //* Only insert at unused KeyPtrSet
                    if(_data[insertPosition].getBitmap(0)){
                        if(shiftPos < insertPosition){
                            for(int i = shiftPos; i < insertPosition; ++i){
                                _data[i] = _data[i+1];
                            }
                        }
                        else{
                            for(int i = shiftPos; i > insertPosition; --i){
                                _data[i] = _data[i-1];
                            }
                        }

                        //* Evaluate for insert a data
                        Evaluation::insert(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, newData);

                        // if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                        //     insertSkyrmion(overwrittenSkyrmion);
                        // }
                    }
                    
                    _data[insertPosition] = newData;

                    if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                        int64_t diff = diffSkyrmion(oldData._data);
                        if(diff > 0){
                            if(migrateSkyrmion > 0){
                                _migrateCounter.count(migrateSkyrmion);
                                diff -= migrateSkyrmion;
                                diff = diff < 0 ? 0 : diff;
                            }
                            _insertCounter.count(diff);
                            insertSkyrmion(diff + overwrittenSkyrmion);
                        }
                        else{
                            _removeCounter.count(-diff);
                            removeSkyrmion(-diff);
                        }
                    }

                    switch (_options.update_mode){
                    case Options::update_function::OVERWRITE:
                        Evaluation::overwrite(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, _removeCounter, _data[insertPosition], newData);
                        break;
                    case Options::update_function::PERMUTATION_WRITE:
                        Evaluation::permutation_write(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, _removeCounter, _data[insertPosition], newData);
                        break;
                    case Options::update_function::PERMUTE_WORD_COUNTER:
                        //TODO evaluation 
                        break;
                    case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                        Evaluation::permute(_options, isLeaf(), _readCounter, _shiftCounter, oldData._data, _data);
                        break;
                    default:
                        throw "undefined update operation";
                        break;
                    }

                    // if(_sideBack != nullptr){
                    //     if(insertSide){
                    //         void *temp = _sideBack;
                    //         _sideBack = (Unit *)_data[insertPosition].getPtr();
                    //         _data[insertPosition].setPtr(temp);
                    //     }
                    //     else if(!insertSide && split){
                    //         int i = 0;
                    //         while(!_data[insertPosition+i+1].getBitmap(0))++i;
                    //         void *temp = _data[insertPosition+i+1].getPtr();
                    //         _data[insertPosition+i+1].setPtr(_data[insertPosition].getPtr());
                    //         _data[insertPosition].setPtr(temp);
                    //     }
                    // }
                }
                else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                    // std::clog << "<log> <Node::insertData()> idx: " << idx << std::endl;
                    // std::clog << "<log> <Node::insertData()> data: " << data << std::endl;
                    // std::clog << "<log> <Node::insertData()> _sideBack: " << _sideBack << std::endl;
                    //* insert into the same pointer, just add the index to the key-point set
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                        if(_data[i].getBitmap(0) && _data[i].getPtr() == data){
                            Size overwrittenSkyrmion = Evaluation::countSkyrmion(_data[i].getKey(1));
                            Node oldData = *this;

                            _data[i].addKey(idx);
                            
                            for(int i = 1; i < _options.dataSize(isLeaf()); ++i){
                                if(_data[i].getBitmap(0) && _data[i-1].getBitmap(1) && _data[i].getKey(0) < _data[i-1].getKey(1)){
                                    Index temp = _data[i].getKey(0);
                                    _data[i].setKey(0, _data[i-1].getKey(1));
                                    _data[i-1].setKey(1, temp);
                                } 
                            }

                            // if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                            //     insertSkyrmion(overwrittenSkyrmion);
                            // }
                            // std::clog << "<log> <Node::insertData()> end with same pointer" << std::endl;

                            if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                                int64_t diff = diffSkyrmion(oldData._data);
                                // std::clog << "diff: " << diff << std::endl;
                                if(diff > 0){
                                    if(migrateSkyrmion > 0){
                                        _migrateCounter.count(migrateSkyrmion);
                                        diff -= migrateSkyrmion;
                                        diff = diff < 0 ? 0 : diff;
                                    }
                                    _insertCounter.count(diff);
                                    insertSkyrmion(diff + overwrittenSkyrmion);
                                }
                                else{
                                    _removeCounter.count(-diff);
                                    removeSkyrmion(-diff);
                                }
                            }

                            return;
                        }
                    }

                    KeyPtrSet newData(_options.kp_length, false);
                    newData.setPtr(data);
                    newData.addKey(idx);

                    uint64_t shiftPos = getShiftPosition();
                    insertPosition = getInsertPosition(idx, shiftPos);

                    Size overwrittenSkyrmion = Evaluation::countSkyrmion(_data[shiftPos]);
                    Node oldData = *this;

                    //* Only insert at unused KeyPtrSet
                    if(_data[insertPosition].getBitmap(0)){
                        if(shiftPos < insertPosition){
                            for(int i = shiftPos; i < insertPosition; ++i){
                                _data[i] = _data[i+1];
                            }
                        }
                        else{
                            for(int i = shiftPos; i > insertPosition; --i){
                                _data[i] = _data[i-1];
                            }
                        }

                        //* Evaluate for insert a data
                        Evaluation::insert(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, newData);
                        // if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                        //     insertSkyrmion(overwrittenSkyrmion);
                        // }
                    }
                    
                    _data[insertPosition] = newData;

                    if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                        int64_t diff = diffSkyrmion(oldData._data);
                        if(diff > 0){
                            if(migrateSkyrmion > 0){
                                _migrateCounter.count(migrateSkyrmion);
                                diff -= migrateSkyrmion;
                                diff = diff < 0 ? 0 : diff;
                            }
                            _insertCounter.count(diff);
                            insertSkyrmion(diff + overwrittenSkyrmion);
                        }
                        else{
                            _removeCounter.count(-diff);
                            removeSkyrmion(-diff);
                        }
                    }
                    
                    for(int i = 1; i < _options.dataSize(isLeaf()); ++i){
                        if(_data[i].getBitmap(0) && _data[i-1].getBitmap(1) && _data[i].getKey(0) < _data[i-1].getKey(1)){
                            Index temp = _data[i].getKey(0);
                            _data[i].setKey(0, _data[i-1].getKey(1));
                            _data[i-1].setKey(1, temp);
                        }
                    }

                    switch (_options.update_mode){
                    case Options::update_function::OVERWRITE:
                        Evaluation::overwrite(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, _removeCounter, _data[insertPosition], newData);
                        break;
                    case Options::update_function::PERMUTATION_WRITE:
                        Evaluation::permutation_write(_options, isLeaf(), _readCounter, _shiftCounter, _insertCounter, _removeCounter, _data[insertPosition], newData); 
                        break;
                    case Options::update_function::PERMUTE_WORD_COUNTER:
                        //TODO evaluation 
                        break;
                    case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                        Evaluation::permute(_options, isLeaf(), _readCounter, _shiftCounter, oldData._data, _data);
                        break;
                    default:
                        throw "undefined update operation";
                        break;
                    }

                    
                }
        
            }
        }

        /**
         * * Insert KeyPtrSet to front
        */
        void insertDataFromFront(const KeyPtrSet &data){
            Offset shiftPos = getShiftPosition();
            Offset insertPos = 0;

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
            else{

            }

            _data[insertPos] = data;
        }

        /**
         * * Insert KeyPtrSet to back
        */
        void insertDataFromBack(const KeyPtrSet &data){
            Offset shiftPos = getShiftPosition();
            Offset insertPos = _options.dataSize(isLeaf()) - 1;

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
            else{

            }

            _data[insertPos] = data;
        }

        void insertSkyrmion(Size n){
            // std::clog << "n 0: " << n << std::endl;
            int b = 0;
            while(n){
                // std::clog << "n 1: " << n << std::endl;
                Offset r = -1;
                for(int i = b; i < _options.dataSize(isLeaf()); ++i){
                    if(!_data[i].isFull()){
                        r = i;
                        break;
                    }
                }
                if(r == -1)
                    return;
                
                Offset k = 0;
                Size v = -1;

                if(!_data[r].getBitmap(0)){
                    v = (Size) _data[r].getPtr();
                    k = 0;
                }
                
                if(v == -1){
                    // std::clog << "r 0: " << r << std::endl;
                    if(!_data[r].getBitmap(0)){
                        v = (Size) _data[r].getKey(0);
                        k = 1;
                    }
                    else if(_data[r].getKeyCapacity() == 2 && !_data[r].getBitmap(1)){
                        v = (Size) _data[r].getKey(1);
                        k = 2;
                    }
                    else{
                        b = r + 1;
                        continue;
                    }
                }
                

                if(Evaluation::countSkyrmion(v) == _options.word_length){
                    continue;
                }

                Size i = 1;
                while(v & i == i && i != 0){
                    i <<= 1;
                }
                v += i;

                if(k == 0){
                    _data[r].setPtr((void *)v);
                }
                else if(k == 1){
                    _data[r].setKey(0, v, false);
                }
                else if(k == 2){
                    _data[r].setKey(1, v, false);
                }

                --n;
                
            }
            // std::clog << "n 0: " << n << std::endl;
        }

        void removeSkyrmion(Size n){
            int b = 0;
            while(n){
                // std::clog << "n 1: " << n << std::endl;
                Offset r = -1;
                for(int i = b; i < _options.dataSize(isLeaf()); ++i){
                    if(!_data[i].isFull()){
                        r = i;
                        break;
                    }
                }
                if(r == -1)
                    return;
                
                Offset k = 0;
                Size v = -1;

                if(!_data[r].getBitmap(0)){
                    v = (Size) _data[r].getPtr();
                    k = 0;
                }
                
                if(v == -1){
                    // std::clog << "r 0: " << r << std::endl;
                    if(!_data[r].getBitmap(0)){
                        v = (Size) _data[r].getKey(0);
                        k = 1;
                    }
                    else if(_data[r].getKeyCapacity() == 2 && !_data[r].getBitmap(1)){
                        v = (Size) _data[r].getKey(1);
                        k = 2;
                    }
                    else{
                        b = r + 1;
                        continue;
                    }
                }
                

                if(Evaluation::countSkyrmion(v) == _options.word_length){
                    continue;
                }

                Size i = 1;
                while(i != 0){
                    if(v & i == i){
                        break;
                    }
                    i <<= 1;
                }
                v -= i;

                if(k == 0){
                    _data[r].setPtr((void *)v);
                }
                else if(k == 1){
                    _data[r].setKey(0, v, false);
                }
                else if(k == 2){
                    _data[r].setKey(1, v, false);
                }

                --n;
                
            }
        }

        int64_t diffSkyrmion(const KeyPtrSet rightData[]){
            int64_t self = 0, right = 0;
            for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                self += Evaluation::countSkyrmion(_data[i]);
                right += Evaluation::countSkyrmion(rightData[i]);
            }

            return self - right;
        }

        /**
         * * Find and delete index 
         * TODO evaluation 
         * TODO general unit parameter 
         * TODO redesign as deleteData(offset, side) to direct delete (still need to check right most) 
         * @param side for internal, if true, and try to delete the right most index, that will delete the side unit
         */
        void deleteData(uint64_t idx, bool side = false){
            // std::clog << "<log> <Node::deleteData()>: " << idx << std::endl;

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
                // for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                //     if(_data[i].getBitmap(0) && idx == _data[i].getKey(0)){
                //         // delete side unit
                //         if(!_isLeaf && isRightMostOffset(i) && side){
                //             connectBackSideUnit((Unit *)_data[i].getPtr());
                //         }
                //         deleteMark(i);
                //         return;
                //     }
                // }
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                if(_isLeaf){
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                        if(_data[i].getBitmap(0) && idx == _data[i].getKey(0)){
                            _data[i].delKey(0);
                            return;
                        }
                    }
                }
                else{
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                        for(int j = 0; j < _options.unit_size; ++j){
                            if(_data[i].getBitmap(j) && idx == _data[i].getKey(j)){
                                _data[i].delKey(j);
                                return;
                            }
                        }
                    }
                }
            }

            throw "<error> <Node::deleteData()> delete error";
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
        void deleteMark(Offset data_offset, Offset unit_offset = 0){
            if(_isLeaf){
                //_data[offset].delKey(0);
                _data[data_offset].delAll();
            }
            else{
                //_data[offset / _options.unit_size].delKey(offset % _options.unit_size);
                //_data[offset].delKey(0);
                _data[data_offset].delKey(unit_offset);
            }
        }

        /**
         * * Done
         * @return offset
        */
        uint64_t getOffsetByIndex(uint64_t idx){
            if(_isLeaf){
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0) && idx == _data[i].getKey(0)){
                        return i;
                    }
                }
            }
            else{
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    for(int j = 0; j < _options.unit_size; ++j){
                        if(_data[i].getBitmap(j) && idx == _data[i].getKey(j)){
                            return i + j;
                        }
                    }
                }
            }
            
            return _options.dataSize(isLeaf()) * _options.unit_size;
        }

        /**
         * * Done
        */
        uint64_t getMinIndex(){
            if(_isLeaf){
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0)){
                        return _data[i].getKey(0);
                    }
                }
            }
            else{
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
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
        Index getMaxIndex(){
            if(_isLeaf){
                for(int i = _options.dataSize(isLeaf()) - 1; i >= 0; --i){
                    if(_data[i].getBitmap(0)){
                        return _data[i].getKey(0);
                    }
                }
            }
            else{
                for(int i = _options.dataSize(isLeaf()) - 1; i >= 0; --i){
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
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0)){
                        return _data[i];
                    }
                }
            }
            else{
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
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
                for(int i = _options.dataSize(isLeaf()) - 1; i >= 0; --i){
                    if(_data[i].getBitmap(0)){
                        return _data[i];
                    }
                }
            }
            else{
                for(int i = _options.dataSize(isLeaf()) - 1; i >= 0; --i){
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
        Offset getLeftMostOffset(){
            if(_isLeaf){
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0))
                        return i;
                }
            }
            else{
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0))
                        return i;
                }
            }

            return -1;
        }

        /**
         * * Done
        */
        Offset getRightMostOffset(){
            if(_isLeaf){
                for(int i = _options.dataSize(isLeaf()) - 1; i >= 0; --i){
                    if(_data[i].getBitmap(0))
                        return i;
                }
            }
            else{
                for(int i = _options.dataSize(isLeaf()) - 1; i >= 0; --i){
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
            if(offset == _options.dataSize(isLeaf()))
                return true;
                
            if(offset == -1)
                return false;

            if(_isLeaf){
                if(!_data[offset].getBitmap(0))
                    return false;

                for(int i = offset + 1; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0))
                        return false;
                }
            }
            else{
                if(!_data[offset / _options.unit_size].getBitmap(offset % _options.unit_size))
                    return false;

                for(int i = offset + 1; i <_options.dataSize(isLeaf()); ++i){
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
            if(offset == -1 || offset == _options.dataSize(isLeaf()))
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
                for(int i = offset + 1; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0)){
                        return i;
                    }
                }
            }
            else{
                for(int i = offset + 1; i < _options.dataSize(isLeaf()); ++i){
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
        Offset getClosestLeftOffset(Offset offset){
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
        Offset getInsertPosition(Index wait_insert_idx, Offset shiftPosition){
            if(_isLeaf){
                if(_options.node_ordering == Options::ordering::SORTED){
                    Offset last = 0;
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                        if(_data[i].getBitmap(0)){
                            last = i + 1;
                        } 
                    }
                    
                    for(int i = 0; i < last; ++i){
                        if(!_data[i].getBitmap(0) && _data[i+1].getBitmap(0) && wait_insert_idx < _data[i+1].getKey(0)){
                            return i;
                        }
                        else if(_data[i].getBitmap(0) && wait_insert_idx < _data[i].getKey(0)){
                            return i > shiftPosition ? i - 1 : i;
                        }
                    }
                    
                    // insertSide = true;
                    return last == _options.dataSize(isLeaf()) ? last - 1 : last;
                }
                else if(_options.node_ordering == Options::ordering::UNSORTED){
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                        if(!_data[i].getBitmap(0)){
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
                Offset last = 0;
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0)){
                        last = i + 1;
                    } 
                }
                
                for(int i = 0; i < last; ++i){
                    if(!_data[i].getBitmap(0) && _data[i+1].getBitmap(0) && wait_insert_idx < _data[i+1].getKey(0)){
                        return i;
                    }
                    else if(_data[i].getBitmap(0) && wait_insert_idx < _data[i].getKey(0)){
                        return i > shiftPosition ? i - 1 : i;
                    }
                }

                // insertSide = true;
                return last == _options.dataSize(isLeaf()) ? last - 1 : last;
            }
            
        }

        Offset getShiftPosition(){
            Offset shiftPoint = -1;

            if(_isLeaf){
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(!_data[i].getBitmap(0)){
                        shiftPoint = i;
                        break;
                    }
                }
            }
            else{
                // for(int i = 0; i < _options.track_length; ++i){
                //     for(int j = 0; j < _options.unit_size; ++j){
                //         if(!_data[i].getBitmap(j)){
                //             shiftPoint = i + j;
                //             break;
                //         }
                //     }
                // }

                // std::clog << "<log> <Node::getShiftPosition()> " << std::endl;
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    // std::clog << _data[i].getKey(0) << " ";
                    // std::clog << _data[i].getBitmap(0) << " ";
                    // std::clog << _data[i].getKey(1) << " ";
                    // std::clog << _data[i].getBitmap(1) << ", ";
                    if(!_data[i].getBitmap(0)){
                        shiftPoint = i;
                        break;
                    }
                }
                // std::clog << std::endl;
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
                return getSize() == _options.dataSize(isLeaf());
            }
            else{
                return getSize() == _options.dataSize(isLeaf());
            }
            
        }

        /**
         * * Done
        */
        bool isHalf() const{
            return getSize() == _options.dataSize(isLeaf()) / 2;
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
        Size getSize() const{
            Size counter = 0;

            if(_isLeaf){
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(_data[i].getBitmap(0)){
                        counter++;
                    }
                }
            }
            else{
                if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
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
                    
                    for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                        if(_data[i].getBitmap(0) || _data[i].getBitmap(1)){
                            counter++;
                        }
                    }
                    
                }
                
            }

            return counter;
        }

        bool isLeaf() const{
            return _isLeaf;
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
        Offset _parentOffset;
        bool _isValid;

        /* System */
        Options _options;
        Size _id;

        Counter _readCounter = Counter("Read");
        Counter _shiftCounter = Counter("Shift");
        Counter _insertCounter = Counter("Insert");
        Counter _removeCounter = Counter("Remove");
        Counter _migrateCounter = Counter("Migrate");
    };

    Unit(Options options, bool isLeaf = true) : _options(options){
        //* init tracks
        _tracks = new Node[_options.unit_size]();
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i] = Node(_options, false, isLeaf);
        }
        _tracks[0].setValid(true);

        //* assume new unit is root
        _isRoot = true;

        //* get unit id
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

    KeyPtrSet *readData(Offset unit_offset){
        return _tracks[unit_offset].readData(0, _options.dataSize(isLeaf()));
    }

    /**
     * * Return data pointer
     * * DONE
     */
    Data *searchData(const Index &idx, Offset &unit_offset){
        Data *dataPtr = nullptr;

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            if(isLeaf()){
                try{
                    dataPtr = (Data *)_tracks[unit_offset].searchData(idx, unit_offset);
                }
                catch(int e){
                    if(e != -1) throw "search error";
                }
            }
            else{
                try{
                    dataPtr = (Data *)((Unit *)_tracks[unit_offset].searchData(idx, unit_offset))->searchData(idx, unit_offset);
                }
                catch(int e){
                    if(e != -1) throw "search error";
                }
            }
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            if(isLeaf()){
                try{
                    //std::clog << "<log> <searchData()> idx: " << idx << std::endl;
                    //std::clog << "<log> <searchData()> _id: " << _id << std::endl;
                    //std::clog << "<log> <searchData()> unit_offset: " << unit_offset << std::endl;
                    dataPtr = (Data *)_tracks[unit_offset].searchData(idx, unit_offset);

                    // if(dataPtr == nullptr){
                    //     if(unit_offset + 1 < _options.unit_size){
                    //         unit_offset += 1;
                    //         dataPtr = (uint64_t *)_tracks[unit_offset].searchData(idx, unit_offset);
                    //     }
                    // }
                    // if(dataPtr == nullptr){
                    //     if(hasBackSideUnit(unit_offset)){
                    //         unit_offset = 0;
                    //         dataPtr = getBackSideUnit()->searchData(idx, unit_offset);
                    //     }
                    // }
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
                    Unit *nextUnit = (Unit *)_tracks[unit_offset].searchData(idx, unit_offset);
                    if(unit_offset == _options.unit_size){
                        unit_offset = 0;
                        // if(nextUnit->_tracks[1].isValid()){
                        //     unit_offset = 1;
                        // }
                        // else{
                        //     unit_offset = 0;
                        // }
                    }
                    dataPtr = (Data *)nextUnit->searchData(idx, unit_offset);
                }
                catch(int e){
                    if(e != -1) throw "search error";
                }
            }
        }

        
        
        return dataPtr;
    }

    void updateData(uint64_t idx, uint64_t data, uint64_t offset){

    }

    /**
     * * Controller of insertion
     * TODO evaluation
     */
    void insertData(Index idx, const Data data, Offset unit_offset, Offset data_enter_offset){
        // std::clog << "<log> <insertData()> idx: " << idx << std::endl;
        if(isLeaf()){
            insertCurrentData(idx, data, unit_offset, data_enter_offset);
        }
        else{
            switch (_options.search_mode){
                case Options::search_function::SEQUENTIAL:
                    //TODO evaluation 
                    break;
                case Options::search_function::TRAD_BINARY_SEARCH:
                    Evaluation::binary_search(_options, isLeaf(), _tracks[unit_offset]._readCounter, _tracks[unit_offset]._shiftCounter, _tracks[unit_offset]._data, idx);
                    break;
                case Options::search_function::BIT_BINARY_SEARCH:
                    Evaluation::bit_binary_search(_options, isLeaf(), _tracks[unit_offset]._readCounter, _tracks[unit_offset]._shiftCounter, _tracks[unit_offset]._data, idx);
                    break;
                default:
                    throw "undefined search operation";
            }

            if(!hasFrontSideUnit(unit_offset)){
                throw "<error> <Unit::insertData()> insert front side error";
            }
            
            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                for(int i = _options.dataSize(isLeaf()) - 1; i >= 0 ; --i){
                    if(
                        _tracks[unit_offset]._data[i].getBitmap(0) &&
                        _tracks[unit_offset]._data[i].getPtr() != nullptr &&
                        idx >= _tracks[unit_offset]._data[i].getKey(0)
                    ){
                        ((Unit *)_tracks[unit_offset]._data[i].getPtr())->insertData(idx, data, 0, i);
                        return;
                    }
                }
                ((Unit *)_tracks[unit_offset]._sideFront)->insertData(idx, data, 0, _options.dataSize(isLeaf()));

                return;
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                for(int i = _options.dataSize(isLeaf()) - 1; i >= 0 ; --i){
                    for(int j = _options.unit_size - 1; j >= 0 ; --j){
                        if(
                            _tracks[unit_offset]._data[i].getBitmap(j) &&
                            _tracks[unit_offset]._data[i].getPtr() != nullptr &&
                            idx >= _tracks[unit_offset]._data[i].getKey(j)
                        ){
                            ((Unit *)_tracks[unit_offset]._data[i].getPtr())->insertData(idx, data, j, i);
                            return;
                        }
                    }
                }
                ((Unit *)_tracks[unit_offset]._sideFront)->insertData(idx, data, 0, _options.dataSize(isLeaf()));

                return;
            }

            throw "<Unit::insertData()> error";
        }
    }

    /**
     * * Insert data to current node
     * ! for leaf node
     * ? can merge with insertCurrentPoint()?
     */
    void insertCurrentData(uint64_t idx, uint64_t data, uint64_t unit_offset, uint64_t data_enter_offset){
        //
        //std::clog << "<log> <insertCurrentData()> data_enter_offset: " << data_enter_offset << std::endl;
        //
        
        Offset insertPosition = -1;

        if(!isLeaf())
            throw "This function is for Leaf node";

        bool insertSide = false;
        if(data_enter_offset == _options.dataSize(isLeaf()) && !isRoot()){
            insertSide = true;
            // data_enter_offset = getParentUnit()->_tracks[getParentOffset()].getRightMostOffset();
        }
        if(!isRoot() && getParentUnit()->getFrontSideUnit(getParentOffset()) == this){
            insertSide = true;
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

                    //* new root unit insert current unit
                    newRoot->connectFrontSideUnit(this);
                    this->connectParentUnit(newRoot, 0);

                    //* new root unit insert new split unit
                    newRoot->insertCurrentPointer(promote.getKey(0), (Unit *)promote.getPtr(), 0, 0);
                    ((Unit *)promote.getPtr())->connectParentUnit(newRoot, 0);

                    // side pointer connect to new split unit
                    connectBackSideUnit((Unit *)promote.ptr);
                    ((Unit *)promote.ptr)->connectFrontSideUnit(this);

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
                getParentUnit()->insertCurrentPointer(promote.getKey(0), (Unit *)promote.getPtr(), getParentOffset(), 0);

                // insert new index from root
                //? or from its parent
                getRoot()->insertData(idx, data, 0, -1);

                return;
            }
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            // std::clog << "<log> <insertCurrentData()> idx: " << idx << std::endl;
            // std::clog << "<log> <insertCurrentData()> _tracks[unit_offset]: " << _tracks[unit_offset] << std::endl;
            // std::clog << "<log> <Unit::insertCurrentData()::UNIT> isFullUnit(): " << isFullUnit() << std::endl;
            // std::clog << "<log> <insertCurrentData()> insertSide: " << insertSide << std::endl;

            if(isFull(unit_offset) && !isFullUnit()){
                if(isRoot()){
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    setRoot(false);

                    // allocate a new root unit
                    Unit *newRoot = System::allocUnit(_options, false);
                    //* it must be internal unit
                    newRoot->setLeaf(false);

                    // new root unit insert current unit
                    newRoot->connectFrontSideUnit(this);
                    this->connectParentUnit(newRoot, 0);

                    // new root unit insert new split unit
                    newRoot->insertCurrentPointer(promote.getKey(0), (Unit *)promote.getPtr(), 0, 0);
                    ((Unit *)promote.getPtr())->connectParentUnit(newRoot, 0);

                    // side pointer connect to new split unit
                    connectBackSideUnit((Unit *)promote.ptr);
                    ((Unit *)promote.ptr)->connectFrontSideUnit(this);

                    // insert new index
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
                else if(insertSide){
                    KeyPtrSet promote = splitNode(idx, unit_offset, insertSide);

                    if(hasBackSideUnit() && promote.getPtr() != getBackSideUnit()){
                        ((Unit *)promote.ptr)->connectBackSideUnit(getBackSideUnit());
                        getBackSideUnit()->connectFrontSideUnit(((Unit *)promote.ptr));
                    }
                    if(promote.getPtr() != this){
                        connectBackSideUnit((Unit *)promote.ptr);
                        ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                    }

                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                    // insert new index from root
                    //? or from its parent
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
                else if(!isAllValid()){
                    //* get the middle key and the pointer of new split unit
                    KeyPtrSet promote = splitNode(idx, unit_offset);

                    if(promote.getKey(0) == 0)
                        return;

                    //std::clog << "<log> this->_tracks[0]._parent: " << this->_tracks[0]._parent << std::endl;
                    getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                    // insert new index from root
                    //? or from its parent
                    getRoot()->insertData(idx, data, 0, -1);

                    return;
                }
                else if(isAllValid() && !isFullUnit()){
                    // std::clog << "<log> <Unit::insertCurrentData()::UNIT> balance" << std::endl;

                    Node *leftNode = nullptr, *rightNode = nullptr;
                    uint64_t mid = 0;
                    // mid = _tracks[unit_offset].getMinIndex();

                    if(unit_offset < (unit_offset+1) % 2){
                        leftNode = &_tracks[unit_offset];
                        rightNode = &_tracks[(unit_offset+1) % 2];
                        balanceDataFromLeft(*leftNode, *rightNode);

                        mid = rightNode->getMinIndex();
                        if(mid < idx && rightNode->isFull()){
                            rightNode->_data[0].setKey(0, idx);
                            swap(mid, idx);
                        }
                    }
                    else{
                        leftNode = &_tracks[(unit_offset+1) % 2];
                        rightNode = &_tracks[unit_offset];
                        balanceDataFromRight(*leftNode, *rightNode);
                        
                        mid = min(idx, rightNode->getMinIndex());
                    }
                    
                    if(idx < mid){
                        leftNode->insertData(idx, new uint64_t(data), insertPosition);
                    }
                    else{
                        rightNode->insertData(idx, new uint64_t(data), insertPosition);
                    }

                    mid = rightNode->getMinIndex();

                    if(!isRoot()){
                        if(insertSide){
                            throw "balance at side";
                        }
                        else{
                            switch (_options.update_mode){
                                case Options::update_function::OVERWRITE:
                                    Evaluation::overwrite(
                                            _options, isLeaf(), 
                                            getParentUnit()->_tracks[getParentOffset()]._readCounter, 
                                            getParentUnit()->_tracks[getParentOffset()]._shiftCounter, 
                                            getParentUnit()->_tracks[getParentOffset()]._insertCounter, 
                                            getParentUnit()->_tracks[getParentOffset()]._removeCounter, 
                                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].getKey(1), 
                                            mid
                                        );
                                    break;
                                case Options::update_function::PERMUTATION_WRITE:
                                    Evaluation::permutation_write(
                                            _options, isLeaf(), 
                                            getParentUnit()->_tracks[getParentOffset()]._readCounter, 
                                            getParentUnit()->_tracks[getParentOffset()]._shiftCounter, 
                                            getParentUnit()->_tracks[getParentOffset()]._insertCounter, 
                                            getParentUnit()->_tracks[getParentOffset()]._removeCounter, 
                                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].getKey(1), 
                                            mid
                                        );
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
                            getParentUnit()->_tracks[getParentOffset()]._data[data_enter_offset].setKey(1, mid);
                        }
                        
                        //std::clog << "<log> <insertCurrentData()> getParentUnit()->_tracks[getParentOffset()]: " << getParentUnit()->_tracks[getParentOffset()] << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode: " << *rightNode << std::endl;
                        //std::clog << "<log> <insertCurrentData()> rightNode->getMinIndex(): " << rightNode->getMinIndex() << std::endl;
                    }
                    
                    return;
                }
                
            }
            else if(isFullUnit()){
                //* get the middle key and the pointer of new split unit
                KeyPtrSet promote = splitNode(idx, unit_offset);

                if(promote.getKey(0) == 0)
                    return;

                // For leaf node, connect two side pointers
                if(hasBackSideUnit() && promote.getPtr() != getBackSideUnit()){
                    ((Unit *)promote.ptr)->connectBackSideUnit(getBackSideUnit());
                    getBackSideUnit()->connectFrontSideUnit(((Unit *)promote.ptr));
                }
                if(promote.getPtr() != this){
                    connectBackSideUnit((Unit *)promote.ptr);
                    ((Unit *)promote.ptr)->connectFrontSideUnit(this);
                }

                //std::clog << "<log> getParentUnit()->_tracks[getParentOffset()]: " << getParentUnit()->_tracks[getParentOffset()] << std::endl;
                getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                // insert new index from root
                //? or from its parent
                getRoot()->insertData(idx, data, 0, -1);

                return;
            }
            
        }

        //* direct insert index and data as data pointer to node
        _tracks[unit_offset].insertData(idx, new Data(data), insertPosition);
        //std::clog << "<log> <insertCurrentData()> _tracks[unit_offset]: " << _tracks[unit_offset] << std::endl;
    }

    void insertCurrentPointer(uint64_t idx, Unit *unit, uint64_t unit_offset, uint64_t data_enter_offset){
        //
        //
        //std::clog << "<log> <insertCurrentPointer()> offset: " << unit_offset << std::endl;
        // 

        Offset insertPosition = -1;

        if(isLeaf())
            throw "This function is for Internal node";
        
        bool insertSide = false;
        if(data_enter_offset == _options.dataSize(isLeaf()) && !isRoot()){
            insertSide = true;
            // data_enter_offset = getParentUnit()->_tracks[getParentOffset()].getRightMostOffset();
        }
        // std::clog << "<log> <insertCurrentPointer()> getParentUnit(): " << getParentUnit() << std::endl;
        if(hasParentUnit() && getParentUnit()->getFrontSideUnit(getParentOffset()) == this){
            insertSide = true;
        }

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            if(isFull(unit_offset)){
                KeyPtrSet promote = splitNode(idx, unit_offset);

                if(_isRoot){
                    setRoot(false);

                    Unit *newRoot = System::allocUnit(_options, false);

                    newRoot->connectFrontSideUnit(this, 0);
                    this->connectParentUnit(newRoot, 0);

                    newRoot->insertCurrentPointer(promote.getKey(0), (Unit *)promote.getPtr(), 0, 0);
                    ((Unit *)promote.getPtr())->connectParentUnit(newRoot, 0);
                    
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
                        // Offset leftMostOffset = rightUnit->_tracks[0].getLeftMostOffset();
                        // rightUnit->_tracks[0]._data[leftMostOffset].setPtr(unit);
                        // unit->connectParentUnit(rightUnit, 0);
                        rightUnit->connectFrontSideUnit(unit);
                        unit->connectParentUnit(rightUnit, 0);
                    }

                    return;
                }
                
                // std::clog << "<log> <insertCurrentPointer()> promote: " << promote << std::endl;
                // std::clog << "<log> <insertCurrentPointer()> promote.getPtr(): " << promote.getPtr() << std::endl;

                Unit *rightUnit = (Unit *)promote.getPtr();
                if(idx < promote.getKey(0)){
                    insertCurrentPointer(idx, unit, 0, 0);
                    unit->connectParentUnit(this, 0);
                }
                else if(idx > promote.getKey(0)){
                    rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                    unit->connectParentUnit(rightUnit, 0);
                }
                else{
                    rightUnit->connectFrontSideUnit(unit);
                    unit->connectParentUnit(rightUnit, 0);
                    
                }
                getParentUnit()->insertCurrentPointer(promote.getKey(0), (Unit *)promote.getPtr(), 0, 0);  

                return;
            }

            _tracks[0].insertData(idx, unit, insertPosition);
            unit->connectParentUnit(this, 0);
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            // std::clog << "<log> <insertCurrentPointer()> idx: " << idx << std::endl;
            // std::clog << "<log> <insertCurrentPointer()> unit: " << unit << std::endl;
            // std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[unit_offset] << std::endl;
            // std::clog << "<log> <insertCurrentPointer()> this: " << this << std::endl;
            // std::clog << "<log> <insertCurrentPointer()> insertSide: " << insertSide << std::endl;
            // std::clog << "<log> <insertCurrentPointer()> isFullUnit(): " << isFullUnit() << std0::endl;
            // std::clog << "<log> <insertCurrentPointer()> isFull(unit_offset): " << isFull(unit_offset) << std::endl;
            // std::clog << "<log> <insertCurrentPointer()> isPossibleInsert(unit_offset, unit): " << isPossibleInsert(unit_offset, unit) << std::endl; 
            
            if(isFull(unit_offset) && !isPossibleInsert(unit_offset, unit)){
                KeyPtrSet promote;
                Unit *rightUnit;
                Offset rightUnitOffset;

                if(isRoot()){
                    // std::clog << "<log> <insertCurrentPointer()> split condition 1" << std::endl;

                    promote = splitNode(idx, unit_offset);

                    setRoot(false);

                    Unit *newRoot = System::allocUnit(_options, false);

                    newRoot->connectFrontSideUnit(this, 0);
                    this->connectParentUnit(newRoot, 0);

                    newRoot->insertCurrentPointer(promote.getKey(0), (Unit *)promote.getPtr(), 0, 0);
                    ((Unit *)promote.getPtr())->connectParentUnit(newRoot, 0);
                    
                    rightUnit = (Unit *)promote.ptr;
                    rightUnitOffset = 0;

                    if(idx < promote.getKey(0)){
                        insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(this, 0);
                    }
                    else if(idx > promote.getKey(0)){
                        rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(rightUnit, 0);
                    }
                    else{ 
                        rightUnit->connectFrontSideUnit(unit);
                        unit->connectParentUnit(rightUnit, 0);
                    }

                    if(rightUnit->getFrontSideUnit(rightUnitOffset)->getValidSize() == 2){
                        // std::clog << "<log> <insertCurrentPointer()> adjust side unit" << std::endl;
                        bool inUnit = rightUnitOffset == 1;
                        Unit *newUnit = System::allocUnit(_options, rightUnit->getFrontSideUnit(rightUnitOffset)->isLeaf());
                        newUnit->setRoot(false);

                        rightUnit->getFrontSideUnit(rightUnitOffset)->migrateNode(rightUnit->getFrontSideUnit(rightUnitOffset)->_tracks[1], newUnit->_tracks[0], newUnit, 0);
                        
                        if(rightUnit->_tracks[rightUnitOffset]._data[0].getKey(0) == idx){
                            swap(rightUnit->_tracks[rightUnitOffset]._data[0].ptr, rightUnit->_tracks[rightUnitOffset]._data[1].ptr);
                        }
                        rightUnit->_tracks[rightUnitOffset]._data[0].setPtr(newUnit);

                        // std::clog << "<log> <insertCurrentPointer()::adjust> idx: " << idx << std::endl;
                        // std::clog << "<log> <insertCurrentPointer()> newUnit: " << newUnit << std::endl;
                        
                        
                        if(inUnit){
                            newUnit->connectParentUnit(rightUnit, 1);
                        }
                        else{
                            newUnit->connectParentUnit(rightUnit, 0);
                        }

                        if(rightUnit->getFrontSideUnit(rightUnitOffset)->isLeaf()){
                            if(rightUnit->getFrontSideUnit(rightUnitOffset)->hasBackSideUnit() && newUnit != rightUnit->getFrontSideUnit(rightUnitOffset)->getBackSideUnit()){
                                newUnit->connectBackSideUnit(rightUnit->getFrontSideUnit(rightUnitOffset)->getBackSideUnit());
                                rightUnit->getFrontSideUnit(rightUnitOffset)->getBackSideUnit()->connectFrontSideUnit(newUnit);
                            }
                            if(newUnit != rightUnit->getFrontSideUnit(rightUnitOffset)){
                                rightUnit->getFrontSideUnit(rightUnitOffset)->connectBackSideUnit(newUnit);
                                newUnit->connectFrontSideUnit(rightUnit->getFrontSideUnit(rightUnitOffset));
                            }
                        }
                        
                    }

                    //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[unit_offset] << std::endl; 
                    return;
                }
                else if(insertSide){
                    // std::clog << "<log> <insertCurrentPointer()> split condition 2" << std::endl;

                    promote = splitNode(idx, unit_offset, insertSide);

                    rightUnit = (Unit *)promote.ptr;
                    rightUnitOffset = 0;

                    if(idx < promote.getKey(0)){
                        insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(this, 0);
                    }
                    else if(idx > promote.getKey(0)){
                        rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(rightUnit, 0);
                    }
                    else{ 
                        rightUnit->connectFrontSideUnit(unit);
                        unit->connectParentUnit(rightUnit, 0);
                    }
                    
                    // getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);          
                    
                    // return;
                }
                else if(!isAllValid()){
                    // std::clog << "<log> <insertCurrentPointer()> split condition 3" << std::endl;
                    
                    promote = splitNode(idx, unit_offset, insertSide);

                    
                    rightUnit = (Unit *)promote.ptr;
                    rightUnitOffset = 1;

                    if(idx < promote.getKey(0)){
                        insertCurrentPointer(idx, unit, 0, 0);
                        unit->connectParentUnit(this, 0);
                    }
                    else if(idx > promote.getKey(0)){
                        insertCurrentPointer(idx, unit, 1, 0);
                        unit->connectParentUnit(this, 1);
                    }
                    else{ 
                        connectFrontSideUnit(unit, 1);
                        unit->connectParentUnit(this, 1);
                    }

                    // getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);          

                    
                    // std::clog << "<log> <insertCurrentPointer()> _tracks[1]: " << _tracks[1] << std::endl;
                    
                    // return;
                }
                else{
                    // throw "test 2";
                    // std::clog << "<log> <insertCurrentPointer()> split condition 4" << std::endl;

                    promote = splitNode(idx, unit_offset, insertSide);

                    if(unit_offset == 0){
                        rightUnit = (Unit *)promote.ptr;
                        rightUnitOffset = 1;

                        if(idx < promote.getKey(0)){
                            // std::clog << "<log> <insertCurrentPointer()> split condition 4a" << std::endl;
                            insertCurrentPointer(idx, unit, 0, 0);
                            unit->connectParentUnit(this, 0);
                        }
                        else if(idx > promote.getKey(0)){
                            // std::clog << "<log> <insertCurrentPointer()> split condition 4b" << std::endl;
                            insertCurrentPointer(idx, unit, 1, 0);
                            unit->connectParentUnit(this, 1);
                        }
                        else{ 
                            // std::clog << "<log> <insertCurrentPointer()> split condition 4c" << std::endl;
                            connectFrontSideUnit(unit, 1);
                            unit->connectParentUnit(this, 1);
                        }
                    }
                    else if(unit_offset == 1){
                        // std::clog << "<log> <insertCurrentPointer()> split condition 5" << std::endl;

                        rightUnit = (Unit *)promote.ptr;
                        rightUnitOffset = 0;

                        if(idx < promote.getKey(0)){
                            insertCurrentPointer(idx, unit, 1, 0);
                            unit->connectParentUnit(this, 1);
                        }
                        else if(idx > promote.getKey(0)){
                            rightUnit->insertCurrentPointer(idx, unit, 0, 0);
                            unit->connectParentUnit(rightUnit, 0);
                        }
                        else{ 
                            rightUnit->connectFrontSideUnit(unit, 0);
                            unit->connectParentUnit(rightUnit, 0);
                        }
                    }
                    
                    // getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);
                    
                    if(unit_offset == 0){
                        rightUnit = this;
                    }
                    // return;
                }

                // std::clog << "<log> <insertCurrentPointer()> rightUnit->hasFrontSideUnit(rightUnitOffset): " << rightUnit->hasFrontSideUnit(rightUnitOffset) << std::endl;
                // std::clog << "<log> <insertCurrentPointer()> rightUnit: " << rightUnit << std::endl;
                // std::clog << "<log> <insertCurrentPointer()> rightUnitOffset: " << rightUnitOffset << std::endl;
                Unit *rightUnitSide = rightUnit->getFrontSideUnit(rightUnitOffset);
                if(rightUnitSide->getValidSize() == 2){
                    // std::clog << "<log> <insertCurrentPointer()> adjust side unit" << std::endl;
                    bool inUnit = rightUnitOffset == 1;
                    Unit *newUnit = System::allocUnit(_options, rightUnitSide->isLeaf());
                    newUnit->setRoot(false);

                    rightUnitSide->migrateNode(rightUnitSide->_tracks[1], newUnit->_tracks[0], newUnit, 0);
                    
                    if(rightUnit->_tracks[rightUnitOffset]._data[0].getKey(0) == idx){
                        swap(rightUnit->_tracks[rightUnitOffset]._data[0].ptr, rightUnit->_tracks[rightUnitOffset]._data[1].ptr);
                    }
                    rightUnit->_tracks[rightUnitOffset]._data[0].setPtr(newUnit);

                    // std::clog << "<log> <insertCurrentPointer()::adjust> idx: " << idx << std::endl;
                    // std::clog << "<log> <insertCurrentPointer()> newUnit: " << newUnit << std::endl;
                    
                    if(inUnit){
                        newUnit->connectParentUnit(rightUnit, 1);
                    }
                    else{
                        newUnit->connectParentUnit(rightUnit, 0);
                    }

                    if(rightUnitSide->isLeaf()){
                        if(rightUnitSide->hasBackSideUnit() && newUnit != rightUnitSide->getBackSideUnit()){
                            newUnit->connectBackSideUnit(rightUnitSide->getBackSideUnit());
                            rightUnitSide->getBackSideUnit()->connectFrontSideUnit(newUnit);
                        }
                        if(newUnit != rightUnitSide){
                            rightUnitSide->connectBackSideUnit(newUnit);
                            newUnit->connectFrontSideUnit(rightUnitSide);
                        }
                    }
                    
                }

                getParentUnit()->insertCurrentPointer(*promote.key, (Unit *)promote.ptr, getParentOffset(), 0);

                return;
            }

            _tracks[unit_offset].insertData(idx, unit, insertPosition);
            unit->connectParentUnit(this, unit_offset);
        }
        
        //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[offset] << std::endl; 
        
        
        
        //std::clog << "<log> <insertCurrentPointer()> _tracks[offset]: " << _tracks[unit_offset] << std::endl; 
    }

    void connectBackSideUnit(Unit *unit, uint64_t unit_offset = 0){
        _tracks[unit_offset].connectBackSideUnit(unit);
    }

    void connectFrontSideUnit(Unit *unit, uint64_t unit_offset = 0){
        _tracks[unit_offset].connectFrontSideUnit(unit);
    }

    bool hasBackSideUnit(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._sideBackBitmap;
    }

    bool hasFrontSideUnit(uint64_t unit_offset) const{
        return _tracks[unit_offset]._sideFrontBitmap;
    }

    Unit *getBackSideUnit(uint64_t unit_offset = 0) const{
        if(!hasBackSideUnit(unit_offset))
            throw "get back side unit error";
        return _tracks[unit_offset]._sideBack;
    }

    Unit *getFrontSideUnit(uint64_t unit_offset) const{
        if(!hasFrontSideUnit(unit_offset))
            throw "get front side unit error";
        return _tracks[unit_offset]._sideFront;
    }

    /**
     * * DONE
    */
    void connectParentUnit(Unit *unit, uint64_t parent_unit_offset = 0, uint64_t unit_offset = -1){
        if(unit_offset == -1){
            for(int i = 0; i < _options.unit_size; ++i){
                _tracks[i].connectParentNode(unit, parent_unit_offset);
            }
        }
        else{
            _tracks[unit_offset].connectParentNode(unit, parent_unit_offset);
        }
        
    }

    bool hasParentUnit(Offset unit_offset = 0) const{
        return _tracks[unit_offset]._parent;
    }

    Unit *getParentUnit(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._parent;
    }

    uint64_t getParentOffset(uint64_t unit_offset = 0) const{
        return _tracks[unit_offset]._parentOffset;
    }

    //? no use
    /*
    Unit *getParentRightUnit(uint64_t idx) const{
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
    KeyPtrSet splitNode(uint64_t &wait_insert_idx, uint64_t unit_offset, bool insertSide = false){
        KeyPtrSet promote(2);

        if(_options.split_merge_mode == Options::split_merge_function::TRAD){
            // split operation will generate same level unit/node
            Unit *newUnit = System::allocUnit(_options, isLeaf());

            // it is never root
            newUnit->setRoot(false);

            // find the middle index
            uint64_t promoteKey = System::getMid(_tracks[0]._data, _options.dataSize(isLeaf()), wait_insert_idx);
            
            // combine as a key-point set
            promote.setPtr(newUnit);
            promote.addKey(promoteKey);
            
            // copying data
            copyHalfNode(_tracks[0], newUnit->_tracks[0], promote, wait_insert_idx);

            // sure to have correct parent pointers
            if(!isLeaf() && newUnit->hasFrontSideUnit(0)){
                getFrontSideUnit(0)->connectParentUnit(this);
                newUnit->getFrontSideUnit(0)->connectParentUnit(newUnit);
            }
        }
        else if(_options.split_merge_mode == Options::split_merge_function::UNIT){ 
            // std::clog << "<log> <splitNode()::UNIT> begin" << std::endl; 
            if(isRoot() || insertSide){
                //* split operation will generate same level unit/node
                Unit *newUnit = System::allocUnit(_options, isLeaf());

                // find the middle index
                uint64_t promoteKey = System::getMid(_tracks[0]._data, _options.dataSize(isLeaf()), wait_insert_idx);

                // combine as a key-point set
                promote.setPtr(newUnit);
                promote.addKey(promoteKey);

                // it is never root
                newUnit->setRoot(false);

                // split operation will generate same level unit/node
                if(!isLeaf()) newUnit->setLeaf(false);

                //* copying data
                copyHalfNode(_tracks[0], newUnit->_tracks[0], promote, wait_insert_idx);
                newUnit->_tracks[0].setValid(true);

                //* sure to have correct parent pointers
                if(!isLeaf() && newUnit->hasFrontSideUnit(0)){
                    getFrontSideUnit(0)->connectParentUnit(this);
                    newUnit->getFrontSideUnit(0)->connectParentUnit(newUnit);
                }
            }
            //* In-Unit Split
            else if(!isAllValid()){
                //* find the middle index
                uint64_t promoteKey = System::getMid(_tracks[0]._data, _options.dataSize(isLeaf()), wait_insert_idx);

                //* combine as a key-point set
                promote.setPtr(this);
                promote.addKey(promoteKey);

                copyHalfNode(_tracks[0], _tracks[1], promote, wait_insert_idx);
                // _tracks[1].setValid(true);  
            }
            //* Unit-Unit Split
            else{
                if(isLeaf()){
                    // split operation will generate same level unit/node
                    Unit *newUnit = System::allocUnit(_options, isLeaf());

                    // find the middle index
                    uint64_t promoteKey = System::getMid(_tracks[1]._data, _options.dataSize(isLeaf()), wait_insert_idx);

                    // combine as a key-point set
                    promote.setPtr(newUnit);
                    promote.addKey(promoteKey);

                    // it is never root
                    newUnit->setRoot(false);

                    // split operation will generate same level unit/node
                    if(!isLeaf()) newUnit->setLeaf(false);

                    //* copying data
                    copyHalfNode(_tracks[1], newUnit->_tracks[0], promote, wait_insert_idx);
                    newUnit->_tracks[0].setValid(true);
                    //std::clog << "<log> newUnit->_tracks[0]: " << newUnit->_tracks[0] << std::endl;
                    
                }
                else{
                    // throw "Internal split developing";
                    // std::clog << "<log> <splitNode()> unit_offset: " << unit_offset << std::endl;
                    if(unit_offset == 0){
                        // throw "<splitNode()::Else> Internal to Leaf Split";
                        // split operation will generate same level unit/node
                        Unit *newUnit = System::allocUnit(_options, isLeaf());

                        // find the middle index
                        Index promoteKey = System::getMid(_tracks[0]._data, _options.dataSize(isLeaf()), wait_insert_idx);

                        // combine as a key-point set
                        promote.setPtr(newUnit);
                        promote.addKey(promoteKey);

                        // it is never root
                        newUnit->setRoot(false);

                        // split operation will generate same level unit/node
                        if(!isLeaf()) newUnit->setLeaf(false);

                        //* copying data
                        migrateNode(_tracks[1], newUnit->_tracks[0], newUnit, 0);
                        promote.setPtr(this);
                        copyHalfNode(_tracks[0], _tracks[1], promote, wait_insert_idx);
                        promote.setPtr(newUnit);
                        // std::clog << "<log> <splitNode()> hasFrontSideUnit(1): " << hasFrontSideUnit(1) << std::endl;

                        //* sure to have correct parent pointers
                        if(!isLeaf() && newUnit->hasFrontSideUnit(0)){
                            getFrontSideUnit(0)->connectParentUnit(this);
                            newUnit->getFrontSideUnit(0)->connectParentUnit(newUnit);
                        }
                        // throw "test 0";
                    }
                    else if(unit_offset == 1){
                        // throw "<splitNode()::Else> Internal to Leaf Split";
                        // split operation will generate same level unit/node
                        Unit *newUnit = System::allocUnit(_options, isLeaf());

                        // find the middle index
                        uint64_t promoteKey = System::getMid(_tracks[1]._data, _options.dataSize(isLeaf()), wait_insert_idx);

                        // combine as a key-point set
                        promote.setPtr(newUnit);
                        promote.addKey(promoteKey);

                        // it is never root
                        newUnit->setRoot(false);

                        // split operation will generate same level unit/node
                        if(!isLeaf()) newUnit->setLeaf(false);

                        //* copying data
                        copyHalfNode(_tracks[1], newUnit->_tracks[0], promote, wait_insert_idx);
                        newUnit->_tracks[0].setValid(true);
                        //std::clog << "<log> newUnit->_tracks[0]: " << newUnit->_tracks[0] << std::endl;

                        //* sure to have correct parent pointers
                        if(!isLeaf() && newUnit->hasFrontSideUnit(1)){
                            getFrontSideUnit(1)->connectParentUnit(this);
                            newUnit->getFrontSideUnit(1)->connectParentUnit(newUnit);
                        }

                    }
                    
                }

            }
            // std::clog << "<log> <splitNode()::UNIT> end" << std::endl; 
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
    void copyHalfNode(Node &source, Node &destination, KeyPtrSet &promote, Index &wait_insert_idx){
        bool inUnit = false;

        if(_options.split_merge_mode == Options::split_merge_function::UNIT){
            inUnit = isFull(0) && !isFull(1) && !isRoot() && getParentUnit()->getFrontSideUnit(getParentOffset()) != this;
        }

        //std::clog << "<log> <copyHalfNode()>" << std::endl;
        switch (_options.read_mode){
            case Options::read_function::SEQUENTIAL:
                Evaluation::sequential_read_half(_options, isLeaf(), source._readCounter, source._shiftCounter);
                break;
            case Options::read_function::RANGE_READ:
                Evaluation::range_read_half(_options, isLeaf(), source._readCounter, source._shiftCounter);
                break;
            default:
                throw "undefined read operation";
                break;
        }

        Node oldData_left = source;
        Node oldData_right = destination;

        if(isLeaf()){
            if(_options.node_ordering == Options::ordering::SORTED){
                Offset mid = 0;
                if(wait_insert_idx < promote.getKey(0)){
                    mid = _options.dataSize(isLeaf()) / 2 - 1;
                }
                else{
                    mid = _options.dataSize(isLeaf()) / 2;
                }

                for(int i = mid, j = 0; i < _options.dataSize(isLeaf()); ++i, ++j){
                    switch (_options.update_mode){
                        case Options::update_function::OVERWRITE:
                            Evaluation::overwrite(
                                    _options, 
                                    isLeaf(), 
                                    destination._readCounter, 
                                    destination._shiftCounter, 
                                    destination._insertCounter, 
                                    destination._removeCounter, 
                                    destination._data[j], source._data[i]
                                );
                            break;
                        case Options::update_function::PERMUTATION_WRITE:
                            Evaluation::permutation_write(
                                    _options, 
                                    isLeaf(), 
                                    destination._readCounter, 
                                    destination._shiftCounter, 
                                    destination._insertCounter, 
                                    destination._removeCounter, 
                                    destination._data[j], source._data[i]
                                );
                            break;
                        case Options::update_function::PERMUTE_WORD_COUNTER:
                            break;
                        case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                            break;
                        default:
                            throw "undefined update operation";
                            break;
                    }

                    destination._data[j] = source._data[i];
                    source._data[i].delAll();
                }

                if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                    switch (_options.read_mode){
                        case Options::read_function::SEQUENTIAL:
                            Evaluation::sequential_read(_options, isLeaf(), destination._readCounter, destination._shiftCounter);
                            break;
                        case Options::read_function::RANGE_READ:
                            Evaluation::range_read(_options, isLeaf(), destination._readCounter, destination._shiftCounter);
                            break;
                        default:
                            throw "undefined read operation";
                            break;
                    }

                    int64_t diff = source.diffSkyrmion(oldData_left._data);
                    if(inUnit){
                        if(diff > 0){
                            // source._migrateCounter.count(diff);
                            // source.insertSkyrmion(diff);
                        }
                        else{
                            // source._removeCounter.count(-diff);
                            source._migrateCounter.count(-diff);
                            source.removeSkyrmion(-diff);
                        }

                        diff = destination.diffSkyrmion(oldData_right._data);
                        if(diff > 0){
                            // destination._migrateCounter.count(diff);
                            // destination.insertSkyrmion(diff);
                        }
                        else{
                            // destination._removeCounter.count(-diff);
                            // destination.removeSkyrmion(-diff);
                        }
                    }
                    else{
                        if(diff > 0){
                            source._insertCounter.count(diff);
                            source.insertSkyrmion(diff);
                        }
                        else{
                            source._removeCounter.count(-diff);
                            source.removeSkyrmion(-diff);
                        }

                        diff = destination.diffSkyrmion(oldData_right._data);
                        if(diff > 0){
                            destination._insertCounter.count(diff);
                            destination.insertSkyrmion(diff);
                        }
                        else{
                            destination._removeCounter.count(-diff);
                            destination.removeSkyrmion(-diff);
                        }
                    }
                }

                switch(_options.update_mode){
                    case Options::update_function::OVERWRITE:
                        break;
                    case Options::update_function::PERMUTATION_WRITE:
                        break;
                    case Options::update_function::PERMUTE_WORD_COUNTER:
                        //TODO evaluation 
                        break;
                    case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                        // Evaluation::permute(
                        //         _options, 
                        //         isLeaf(), 
                        //         source._readCounter, 
                        //         source._shiftCounter, 
                        //         oldData_left._data, source._data
                        //     );
                        // Evaluation::permute(
                        //         _options, 
                        //         isLeaf(), 
                        //         destination._readCounter, 
                        //         destination._shiftCounter, 
                        //         oldData_right._data, destination._data
                        //     );
                        break;
                    default:
                        throw "undefined update operation";
                        break;
                }
            }
            else if(_options.node_ordering == Options::ordering::UNSORTED){
                throw "unsorted split developing";
            }

            destination.setValid(true);

            // std::clog << "<log> <copyHalfNode()> Leaf source: " << source << std::endl;
            // std::clog << "<log> <copyHalfNode()> Leaf destination: " << destination << std::endl;
        }
        else{
            if(_options.split_merge_mode == Options::split_merge_function::TRAD){
                Offset mid = 0;
                if(wait_insert_idx < promote.getKey(0)){
                    mid = _options.dataSize(isLeaf()) / 2 - 1;
                }
                else{
                    mid = _options.dataSize(isLeaf()) / 2;
                }

                for(int i = mid, j = 0; i < _options.dataSize(isLeaf()); ++i, ++j){
                    switch (_options.update_mode){
                        case Options::update_function::OVERWRITE:
                            Evaluation::overwrite(_options, isLeaf(), destination._readCounter, destination._shiftCounter, destination._insertCounter, destination._removeCounter, destination._data[j], source._data[i]);
                            break;
                        case Options::update_function::PERMUTATION_WRITE:
                            Evaluation::permutation_write(_options, isLeaf(), destination._readCounter, destination._shiftCounter, destination._insertCounter, destination._removeCounter, destination._data[j], source._data[i]);
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

                    destination._data[j] = source._data[i];
                    source._data[i].delAll();

                    ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr());
                }
                // destination.connectBackSideUnit(source._sideBack);

                bool promoteMid = true;
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(source._data[i].getBitmap(0) && source._data[i].getKey(0) == promote.getKey(0)){
                        source.deleteMark(i);
                        promoteMid = false;
                        destination.connectFrontSideUnit((Unit *)source._data[i].getPtr());
                        //std::clog << "<log> <copyHalfNode()> source" << std::endl;
                    }
                }
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(destination._data[i].getBitmap(0) && destination._data[i].getKey(0) == promote.getKey(0)){
                        destination.deleteMark(i);
                        promoteMid = false;
                        destination.connectFrontSideUnit((Unit *)destination._data[i].getPtr());
                        //std::clog << "<log> <copyHalfNode()> destination" << std::endl;
                    }                 
                }

                if(promoteMid){
                    //std::clog << "<log> <copyHalfNode()> promoteMid" << std::endl;
                }

                
            }
            else if(_options.split_merge_mode == Options::split_merge_function::UNIT){
                inUnit = isFull(0) && !isFull(1) && !isRoot() && getParentUnit()->getFrontSideUnit(getParentOffset()) != this;
                // std::clog << "<log> <copyHalfNode()> Internal begin" << std::endl;
                // std::clog << "<log> <copyHalfNode()> Internal source begin: " << source << std::endl;
                // std::clog << "<log> <copyHalfNode()> Internal destination begin: " << destination << std::endl;  

                Offset mid = 0;
                if(wait_insert_idx < promote.getKey(0)){
                    mid = _options.dataSize(isLeaf()) / 2 - 1;
                }
                else{
                    mid = _options.dataSize(isLeaf()) / 2;
                }

                for(int i = mid, j = 0; i < _options.dataSize(isLeaf()); ++i, ++j){
                    switch (_options.update_mode){
                        case Options::update_function::OVERWRITE:
                            Evaluation::overwrite(_options, isLeaf(), destination._readCounter, destination._shiftCounter, destination._insertCounter, destination._removeCounter, destination._data[j], source._data[i]);
                            break;
                        case Options::update_function::PERMUTATION_WRITE:
                            Evaluation::permutation_write(_options, isLeaf(), destination._readCounter, destination._shiftCounter, destination._insertCounter, destination._removeCounter, destination._data[j], source._data[i]);
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

                    destination._data[j] = source._data[i];
                    source._data[i].delAll();

                    if(inUnit){
                        ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr(), 1);
                    }
                    else{
                        ((Unit *)destination._data[j].getPtr())->connectParentUnit((Unit *)promote.getPtr(), 0);
                    }
                }
                // destination.connectFrontSideUnit(source._sideFront);
                // if(inUnit){
                //     destination._sideFront->connectParentUnit((Unit *)promote.getPtr(), 1);
                // }
                // else{
                //     destination._sideFront->connectParentUnit((Unit *)promote.getPtr(), 0);
                // }

                bool promoteMid = true;
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    if(source._data[i].getBitmap(0) && source._data[i].getKey(0) == promote.getKey(0)){
                        // std::clog << "<log> <copyHalfNode()> promoteLeft" << std::endl;
                        source.deleteMark(i);
                        promoteMid = false;
                        destination.connectFrontSideUnit((Unit *)source._data[i].getPtr());
                        //std::clog << "<log> <copyHalfNode()> source" << std::endl;
                    }
                }
                for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                    // std::clog << "<log> <copyHalfNode()> " << destination._data[i] << std::endl;
                    if(destination._data[i].getBitmap(0) && destination._data[i].getKey(0) == promote.getKey(0)){
                        // std::clog << "<log> <copyHalfNode()> promoteRight" << std::endl;
                        destination.deleteMark(i);
                        promoteMid = false;
                        destination.connectFrontSideUnit((Unit *)destination._data[i].getPtr());
                    }                 
                }

                if(promoteMid){
                    // std::clog << "<log> <copyHalfNode()> promoteMid" << std::endl;
                    if(promote.getKey(0) < source.getMaxIndex()){
                        Index temp = promote.getKey(0);
                        promote.setKey(0, source._data[source.getRightMostOffset()].getKey(1));
                        source._data[source.getRightMostOffset()].setKey(1, temp);
                        wait_insert_idx = promote.getKey(0);
                        // std::clog << "<log> <copyHalfNode()> wait_insert_idx: " << wait_insert_idx << std::endl;
                    }
                    //std::clog << "<log> <copyHalfNode()> destination._data[0].getPtr(): " << destination._data[0].getPtr() << std::endl;
                }

                destination.setValid(true);

                // std::clog << "<log> <copyHalfNode()> Internal to Leaf source end: " << source << std::endl;
                // std::clog << "<log> <copyHalfNode()> Internal to Leaf destination end: " << destination << std::endl;
                // std::clog << "<log> <copyHalfNode()> Internal end" << std::endl;
            }

            if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
                switch (_options.read_mode){
                    case Options::read_function::SEQUENTIAL:
                        Evaluation::sequential_read(_options, isLeaf(), destination._readCounter, destination._shiftCounter);
                        break;
                    case Options::read_function::RANGE_READ:
                        Evaluation::range_read(_options, isLeaf(), destination._readCounter, destination._shiftCounter);
                        break;
                    default:
                        throw "undefined read operation";
                        break;
                }

                int64_t diff = source.diffSkyrmion(oldData_left._data);
                if(inUnit){
                    if(diff > 0){
                        // source._migrateCounter.count(diff);
                        // source.insertSkyrmion(diff);
                    }
                    else{
                        // source._removeCounter.count(-diff);
                        source._migrateCounter.count(-diff);
                        source.removeSkyrmion(-diff);
                    }

                    diff = destination.diffSkyrmion(oldData_right._data);
                    if(diff > 0){
                        // destination._migrateCounter.count(diff);
                        // destination.insertSkyrmion(diff);
                    }
                    else{
                        // destination._removeCounter.count(-diff);
                        // destination.removeSkyrmion(-diff);
                    }
                }
                else{
                    if(diff > 0){
                        source._insertCounter.count(diff);
                        source.insertSkyrmion(diff);
                    }
                    else{
                        source._removeCounter.count(-diff);
                        source.removeSkyrmion(-diff);
                    }

                    diff = destination.diffSkyrmion(oldData_right._data);
                    if(diff > 0){
                        destination._insertCounter.count(diff);
                        destination.insertSkyrmion(diff);
                    }
                    else{
                        destination._removeCounter.count(-diff);
                        destination.removeSkyrmion(-diff);
                    }
                }
                
            }
            
            switch(_options.update_mode){
                case Options::update_function::OVERWRITE:
                    break;
                case Options::update_function::PERMUTATION_WRITE:
                    break;
                case Options::update_function::PERMUTE_WORD_COUNTER:
                    //TODO evaluation 
                    break;
                case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                    // Evaluation::permute(
                    //         _options, 
                    //         isLeaf(), 
                    //         source._readCounter, 
                    //         source._shiftCounter, 
                    //         oldData_left._data, source._data
                    //     );
                    // Evaluation::permute(
                    //         _options, 
                    //         isLeaf(), 
                    //         destination._readCounter, 
                    //         destination._shiftCounter, 
                    //         oldData_right._data, destination._data
                    //     );
                    break;
                default:
                    throw "undefined update operation";
                    break;
            }
        }
    }
    
    /**
     * * control data deletion
     * TODO evaluation 
     * TODO general unit parameter
     */
    void deleteData(uint64_t idx, uint64_t unit_offset, uint64_t enter_offset, bool &mergeFlag){
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
                            uint64_t deleteIndex = _tracks[i]._data[j].getKey(0);
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
                    uint64_t deleteOffset = _tracks[unit_offset].getRightMostOffset();
                    uint64_t deleteIndex = _tracks[unit_offset]._data[deleteOffset].getKey(0);
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
    void deleteCurrentData(uint64_t idx, uint64_t unit_offset, uint64_t data_offset, uint64_t enter_offset, bool &mergeFlag){
        Offset insertPosition = -1;
        
        //std::clog << "<log> <deleteCurrentData()> idx: " << idx << std::endl;
        //std::clog << "<log> <deleteCurrentData()> _id: " << _id << std::endl;
        //std::clog << "<log> <deleteCurrentData()> begin" << std::endl;
        //std::clog << "<log> data_offset: " << data_offset << std::endl;
        //std::clog << "<log> enter_offset: " << enter_offset << std::endl;

        if(_isRoot){
            //std::clog << "<log> delete at root" << std::endl;
            bool dataSide = false;
            if(data_offset == _options.dataSize(isLeaf())){
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

            if(data_offset == _options.dataSize(isLeaf()))
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

                        uint64_t cloestLeftOffset = getParentUnit()->_tracks[0].getClosestLeftOffset(enter_offset);
                        uint64_t idxFromParent = getParentUnit()->_tracks[0]._data[cloestLeftOffset].getKey(0);
                        //
                        selfUnit->_tracks[unit_offset].deleteData(idx, side);
                        leftUnit->_tracks[unit_offset].insertData(idxFromParent, leftUnit->getBackSideUnit(), insertPosition, false);
                        
                        //data_offset = selfUnit->_tracks[unit_offset].getOffsetByIndex(idx);
                        //selfUnit->_tracks[unit_offset]._data[data_offset].setKey(0, idxFromParent);
                        mergeNodeFromRight(*leftUnit, *selfUnit);
                        leftUnit->connectBackSideUnit(selfUnit->getBackSideUnit());
                        mergeFlag = true;
                        
                        //std::clog << "<log> cloestLeftOffset: " << cloestLeftOffset << std::endl; 
                        
                        
                        //std::clog << "<log> idxFromParent: " << idxFromParent << std::endl; 
                        //std::clog << "<log> getParentUnit()->_tracks[0]: " << getParentUnit()->_tracks[0] << std::endl; 
                        if(enter_offset == _options.dataSize(isLeaf())){
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
                        uint64_t cloestLeftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(enter_offset);
                        uint64_t idxFromParent = getParentUnit()->_tracks[unit_offset]._data[cloestLeftOffset].getKey(0);
                        getParentUnit()->_tracks[unit_offset]._data[cloestLeftOffset].setKey(0, borrow.getKey(0));
                        //getParentUnit()->_tracks[unit_offset]._data[data_offset-1].setKey(0, selfUnit->_tracks[0].getMinIndex());
                        uint64_t deleteOffset = _tracks[unit_offset].getOffsetByIndex(idx);
                        _tracks[unit_offset].deleteData(idx, side); //? side 
                        {
                            uint64_t cloestLeftOffset = _tracks[unit_offset].getClosestLeftOffset(deleteOffset);
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

                        //uint64_t leftMostOffset = getParentUnit()->_tracks[0].getLeftMostOffset();
                        uint64_t idxFromParent = getParentUnit()->_tracks[0]._data[enter_offset].getKey(0);
                        rightUnit->_tracks[unit_offset].insertData(idxFromParent, selfUnit->getBackSideUnit(), insertPosition, false);
                    }
                    else{
                        getParentUnit()->_tracks[unit_offset]._data[enter_offset].setKey(0, borrow.getKey(0));
                        _tracks[unit_offset].deleteData(idx, side);
                        _tracks[unit_offset].insertData(borrow.getKey(0), borrow.getPtr(), insertPosition);
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
        Offset insertPosition = -1;
        //! left_unit_offset = 
        //! right_unit_offset = 
        
        if(right.isLeaf() && !right.isHalf(0)){
            KeyPtrSet borrow = right._tracks[0].getMinData();
            right._tracks[0].deleteData(right._tracks[0].getMinIndex());
            left._tracks[0].insertData(borrow.getKey(0), borrow.getPtr(), insertPosition);
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
        Offset insertPosition = -1;
        //! left_unit_offset = 
        //! right_unit_offset = 

        if(left.isLeaf() && !left.isHalf(0)){
            KeyPtrSet borrow = left._tracks[0].getMaxData();
            left._tracks[0].deleteData(left._tracks[0].getMaxIndex());
            right._tracks[0].insertData(borrow.getKey(0), borrow.getPtr(), insertPosition);
            return borrow;
        }
        else if(!left.isLeaf() && !left.isHalf(0)){
            KeyPtrSet borrow = left._tracks[0].getMaxData();
            borrow.setPtr(left.getBackSideUnit());
            ((Unit *)borrow.getPtr())->connectParentUnit(&right);
            left._tracks[0].deleteData(left._tracks[0].getMaxIndex(), true);
            right._tracks[0].insertData(borrow.getKey(0), borrow.getPtr(), insertPosition, false);

            return borrow;
        }

        return KeyPtrSet();
    }

    // Merge right Node to left Node
    void mergeNodeFromRight(Unit &left, Unit &right){
        Offset insertPosition = -1;
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
            for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                if(right._tracks[0]._data[i].getBitmap(0)){ //TODO
                    left._tracks[0].insertData(right._tracks[0]._data[i].getKey(0), right._tracks[0]._data[i].getPtr(), insertPosition);
                    right._tracks[0].deleteMark(i);
                }
            }
        }
        else{
            Unit *tempSide = left.getBackSideUnit();
            left.connectBackSideUnit(nullptr);
            bool first = true;
            for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                if(right._tracks[0]._data[i].getBitmap(0)){ //TODO
                    ((Unit *)right._tracks[0]._data[i].getPtr())->connectParentUnit(&left);
                    left._tracks[0].insertData(right._tracks[0]._data[i].getKey(0), right._tracks[0]._data[i].getPtr(), insertPosition, false);
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
        Offset insertPosition = -1;
        //! left_unit_offset = 
        //! right_unit_offset = 
        //std::clog << "<log> <mergeNodeFromLeft()> begin left: " << left._tracks[0] << std::endl;
        //std::clog << "<log> <mergeNodeFromLeft()> begin right: " << right._tracks[0] << std::endl;
        
        if(left.isLeaf()){
            for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                if(left._tracks[0]._data[i].getBitmap(0)){ //TODO
                    right._tracks[0].insertData(left._tracks[0]._data[i].getKey(0), left._tracks[0]._data[i].getPtr(), insertPosition);
                    left._tracks[0].deleteMark(i);
                }
            }
            //std::clog << "<log> left: " << left << std::endl;
            //std::clog << "<log> right: " << right << std::endl;
        }
        else{
            //left.connectSideUnit(right._side);
            for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                if(left._tracks[0]._data[i].getBitmap(0)){ //TODO
                    ((Unit *)left._tracks[0]._data[i].getPtr())->connectParentUnit(&right);
                    right._tracks[0].insertData(left._tracks[0]._data[i].getKey(0), left._tracks[0]._data[i].getPtr(), insertPosition, false);
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
        Offset insertPosition = -1;

        if(isLeaf()){
            if(right.getSize() - left.getSize() == 1){
                left.insertData(right._data[0].getKey(0), right._data[0].getPtr(), insertPosition, true, Evaluation::countSkyrmion(right._data[0]));
                // left.insertDataFromBack(right._data[0]);
                
                right.deleteMark(0);
                right.removeSkyrmion(Evaluation::countSkyrmion(right._data[0]));
            }
            else{
                for(int i = 0; i < (right.getSize() - left.getSize()) / 2 ; ++i){
                    left.insertData(right._data[i].getKey(0), right._data[i].getPtr(), insertPosition, true, Evaluation::countSkyrmion(right._data[0]));
                    // left.insertDataFromBack(right._data[i]);
                    
                    right.deleteData(right._data[i].getKey(0));
                    right.removeSkyrmion(Evaluation::countSkyrmion(right._data[i]));
                }
            }
        }
        else{
            // if(right.getSize() - left.getSize() == 1){
            //     ((Unit *)right._data[0].getPtr())->connectParentUnit(this, 0);
            //     left.insertDataFromBack(right._data[0]);

            //     uint64_t rightMostoffset = left.getRightMostOffset();
            //     void *temp = left._sideBack;
            //     left.connectBackSideUnit((Unit *)left._data[rightMostoffset].getPtr());
            //     left._data[rightMostoffset].setPtr(temp);

            //     right.deleteMark(0);
            // }
            // else{
            //     for(int i = 0; i < (right.getSize() - left.getSize()) / 2; ++i){
            //         ((Unit *)right._data[i].getPtr())->connectParentUnit(this, 0);
            //         left.insertDataFromBack(right._data[i]);

            //         uint64_t rightMostoffset = left.getRightMostOffset();
            //         void *temp = left._sideBack;
            //         left.connectBackSideUnit((Unit *)left._data[rightMostoffset].getPtr());
            //         left._data[rightMostoffset].setPtr(temp);

            //         right.deleteMark(i);
            //     }
            // }
        }

        
    }

    /**
     * * for double tracks
     * @param left is full node
     * @param right is non-full node
    */
    void balanceDataFromLeft(Node &left, Node &right, KeyPtrSet promote = KeyPtrSet()){
        Offset insertPosition = -1;
        Offset begin = (left.getSize() + right.getSize()) / 2;

        if(isLeaf()){
            for(int i = begin; i < _options.dataSize(isLeaf()); ++i){
                right.insertData(left._data[i].getKey(0), left._data[i].getPtr(), insertPosition, true, Evaluation::countSkyrmion(left._data[i]));
                // right.insertDataFromFront(left._data[i]);
                
                left.deleteData(left._data[i].getKey(0));
                left.removeSkyrmion(Evaluation::countSkyrmion(left._data[i]));
            }

            //std::clog << "<log> <balanceDataFromLeft()> Leaf left: " << left << std::endl;
            //std::clog << "<log> <balanceDataFromLeft()> Leaf right: " << right << std::endl;
        }
        else{
            // Offset rightMostOffset = left.getRightMostOffset();
            // Index maxIndex = left.getMaxIndex();

            // if(left._sideBack->getValidSize() == 1){
            //     left._sideBack->connectParentUnit(this, 1);
            //     right.insertData(maxIndex, left._sideBack, insertPosition, false);
            //     left.deleteData(maxIndex);
            //     left.connectBackSideUnit((Unit *)left._data[left.getRightMostOffset()].getPtr());
            // }
            // else if(left._sideBack->getParentOffset(1) != left._sideBack->getParentOffset(0)){
            //     left._sideBack->connectParentUnit(this, 1);
            //     right.insertData(maxIndex, left._sideBack, insertPosition, false);
            //     left.deleteData(maxIndex);
            //     left.connectBackSideUnit((Unit *)left._data[left.getRightMostOffset()].getPtr());
            // }
            // else{
            //     left._sideBack->connectParentUnit(this, 1, 1);
            //     right.insertData(maxIndex, left._sideBack, insertPosition, false);
            //     left.deleteData(maxIndex);
            // }

            // std::clog << "<log> <balanceDataFromLeft()> Internal left: " << left << std::endl;
            // std::clog << "<log> <balanceDataFromLeft()> Internal right: " << right << std::endl;
        }

        
    }

    /**
     * * for double tracks
     * * From left to right
     * @param left
     * @param right
    */
    void migrateNode(Node &left, Node &right, Unit *parentUnit = nullptr, unsigned parentUnitOffset = 0){
        if(left._isLeaf != right._isLeaf){
            throw "<migrateNode()> type error";
        }

        bool inUnit = left._parent == right._parent;

        //TODO Evaluation::migrate() need to check inUnit

        switch (_options.read_mode){
            case Options::read_function::SEQUENTIAL:
                Evaluation::sequential_read(_options, isLeaf(), left._readCounter, left._shiftCounter);
                break;
            case Options::read_function::RANGE_READ:
                Evaluation::range_read(_options, isLeaf(), left._readCounter, left._shiftCounter);
                break;
            default:
                throw "undefined read operation";
                break;
        }

        Node oldData_left = left;
        Node oldData_right = right;

        // std::clog << "<log> <Unit::migrateNode()>" << std::endl;
        // std::clog << "<log> <migrateNode() begin> left: " << left << std::endl;
        // std::clog << "<log> <migrateNode() begin> right: " << right << std::endl;

        if(left._isLeaf){
            for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                if(left._data[i].getBitmap(0)){
                    switch (_options.update_mode){
                        case Options::update_function::OVERWRITE:
                            Evaluation::overwrite(
                                _options, 
                                isLeaf(), 
                                right._readCounter, 
                                right._shiftCounter, 
                                right._insertCounter, 
                                right._removeCounter, 
                                right._data[i], left._data[i]
                            );
                            break;
                        case Options::update_function::PERMUTATION_WRITE:
                            Evaluation::permutation_write(
                                _options, 
                                isLeaf(), 
                                right._readCounter, 
                                right._shiftCounter, 
                                right._insertCounter, 
                                right._removeCounter, 
                                right._data[i], left._data[i]
                            );
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

                    right._data[i] = left._data[i];
                    left.deleteMark(i);
                }
            }
        }
        else{
            if(parentUnit == nullptr){
                throw "<Unit::migrateNode()::Internal> error";
            }

            for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                if(left._data[i].getBitmap(0)){
                    switch (_options.update_mode){
                        case Options::update_function::OVERWRITE:
                            Evaluation::overwrite(
                                _options, 
                                isLeaf(), 
                                right._readCounter, 
                                right._shiftCounter, 
                                right._insertCounter, 
                                right._removeCounter, 
                                right._data[i], left._data[i]
                            );
                            break;
                        case Options::update_function::PERMUTATION_WRITE:
                            Evaluation::permutation_write(
                                _options, 
                                isLeaf(), 
                                right._readCounter, 
                                right._shiftCounter, 
                                right._insertCounter, 
                                right._removeCounter, 
                                right._data[i], left._data[i]
                            );
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

                    right._data[i] = left._data[i];
                    if((Unit *)right._data[i].getPtr() != nullptr)
                        ((Unit *)right._data[i].getPtr())->connectParentUnit(parentUnit, parentUnitOffset);
                    left._data[i].delAll();
                    // std::clog << right._data[i] << std::endl;
                }
            }
            right.connectFrontSideUnit(left._sideFront);
            right._sideFront->connectParentUnit(parentUnit, parentUnitOffset);
            // std::clog << "side: " << right._sideFront << std::endl;
        }

        if(_options.update_mode == Options::update_function::PERMUTE_WITHOUT_COUNTER){
            switch (_options.read_mode){
                case Options::read_function::SEQUENTIAL:
                    Evaluation::sequential_read(_options, isLeaf(), right._readCounter, right._shiftCounter);
                    break;
                case Options::read_function::RANGE_READ:
                    Evaluation::range_read(_options, isLeaf(), right._readCounter, right._shiftCounter);
                    break;
                default:
                    throw "undefined read operation";
                    break;
            }

            int64_t diff = left.diffSkyrmion(oldData_left._data);
            if(inUnit){
                if(diff > 0){
                    // source._migrateCounter.count(diff);
                    // source.insertSkyrmion(diff);
                }
                else{
                    // source._removeCounter.count(-diff);
                    left._migrateCounter.count(-diff);
                    left.removeSkyrmion(-diff);
                }

                diff = right.diffSkyrmion(oldData_right._data);
                if(diff > 0){
                    // right._migrateCounter.count(diff);
                    // destination.insertSkyrmion(diff);
                }
                else{
                    // destination._removeCounter.count(-diff);
                    // destination.removeSkyrmion(-diff);
                }
            }
            else{
                if(diff > 0){
                    left._insertCounter.count(diff);
                    left.insertSkyrmion(diff);
                }
                else{
                    left._removeCounter.count(-diff);
                    left.removeSkyrmion(-diff);
                }

                diff = right.diffSkyrmion(oldData_right._data);
                if(diff > 0){
                    right._insertCounter.count(diff);
                    right.insertSkyrmion(diff);
                }
                else{
                    right._removeCounter.count(-diff);
                    right.removeSkyrmion(-diff);
                }
            }
        }
        
        switch(_options.update_mode){
            case Options::update_function::OVERWRITE:
                break;
            case Options::update_function::PERMUTATION_WRITE:
                break;
            case Options::update_function::PERMUTE_WORD_COUNTER:
                //TODO evaluation 
                break;
            case Options::update_function::PERMUTE_WITHOUT_COUNTER:
                // Evaluation::permute(
                //         _options, 
                //         isLeaf(), 
                //         left._readCounter, 
                //         left._shiftCounter, 
                //         oldData_left._data, left._data
                //     );
                // Evaluation::permute(
                //         _options, 
                //         isLeaf(), 
                //         right._readCounter, 
                //         right._shiftCounter, 
                //         oldData_right._data, right._data
                //     );
                break;
            default:
                throw "undefined update operation";
                break;
        }


        left.setValid(false);
        right.setValid(true);

        // std::clog << "<log> <migrateNode() end> left: " << left << std::endl;
        // std::clog << "<log> <migrateNode() end> right: " << right << std::endl;
    }

    /* Minor Functions */

    Unit *findRightUnit(uint64_t unit_offset, uint64_t data_offset, uint64_t enter_offset){
        //std::clog << "<log> enter_offset: " << enter_offset << std::endl;
        //std::clog << "<log> data_offset: " << data_offset << std::endl;
        
        if(_isRoot)
            return nullptr;

        if(enter_offset == _options.dataSize(isLeaf())){
            return nullptr;
        }

        if(getParentUnit()->_tracks[unit_offset].isRightMostOffset(enter_offset)){
            return getParentUnit()->getBackSideUnit();
        }

        if(!getParentUnit()->_tracks[unit_offset].isRightMostOffset(enter_offset)){
            uint64_t rightOffset = getParentUnit()->_tracks[unit_offset].getClosestRightOffset(enter_offset);
            return (Unit *)getParentUnit()->_tracks[unit_offset]._data[rightOffset].getPtr();
            //std::clog << "<log> rightUnit: " << rightUnit << std::endl;
        }

        return nullptr;
    }

    Unit *findLeftUnit(uint64_t unit_offset, uint64_t data_offset, uint64_t enter_offset){
        if(_isRoot)
            return nullptr;
        
        if(!getParentUnit()->_tracks[unit_offset].isLeftMostOffset(enter_offset)){
            uint64_t leftOffset = getParentUnit()->_tracks[unit_offset].getClosestLeftOffset(enter_offset);
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
    void adjInternalIndex(uint64_t unit_offset = 0){
        //std::clog << "<log> <adjInternalIndex()> begin" << std::endl;
        if(!isLeaf()){
            for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
                for(int j = 0; j < _options.unit_size; ++j){
                    if(_tracks[unit_offset]._data[i].getBitmap(j)){ //TODO
                        ((Unit *)_tracks[unit_offset]._data[i].getPtr())->adjInternalIndex(j);

                        Unit *nextUnit = nullptr;
                        //std::clog << "<log> <adjInternalIndex()> nextUnit: " << nextUnit << std::endl;
                        if(_tracks[unit_offset].isRightMostOffset(i)){
                            nextUnit = getBackSideUnit(unit_offset);
                        }
                        else{
                            uint64_t cloestRightOffset = _tracks[unit_offset].getClosestRightOffset(i);
                            nextUnit = (Unit *)_tracks[unit_offset]._data[cloestRightOffset / _options.unit_size].getPtr();
                        }
                        //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0]: " << nextUnit->_tracks[0] << std::endl;
                        //std::clog << "<log> <adjInternalIndex()> nextUnit: " << nextUnit << std::endl;

                        if(nextUnit->isLeaf()){
                            _tracks[unit_offset]._data[i].setKey(j, nextUnit->_tracks[j].getMinIndex());
                            //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0].getMinIndex(): " << nextUnit->_tracks[0].getMinIndex() << std::endl;
                        }
                        else{
                            while(!nextUnit->isLeaf()){
                                //std::clog << "<log> <adjInternalIndex()> nextUnit->_tracks[0] ori: " << nextUnit->_tracks[0] << std::endl;
                                uint64_t leftMostOffset = nextUnit->_tracks[j].getLeftMostOffset();
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

    bool isFull(uint64_t offset) const{
        return _tracks[offset].isFull();
    }

    bool isFullUnit() const{
        for(int i = 0; i < _options.unit_size; ++i){
            if(!isValid(i) || !isFull(i)){
                return false;
            }
        }
        return true;
    }

    // For root unit to check empty
    bool isEmpty(uint64_t offset){
        for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
            if(_tracks[offset]._data[i].getBitmap(0)){ //TODO
                return false;
            }
        }
        return true;
    }

    bool isHalf(uint64_t offset) const{
        return _tracks[offset].isHalf();
    }

    bool isLeaf() const{
        return _tracks[0]._isLeaf;
    }

    bool isValid(uint64_t offset) const{
        return _tracks[offset].isValid();
    }

    bool isAllValid() const{
        for(int i = 0; i < _options.unit_size; ++i){
            if(!_tracks[i].isValid()){
                return false;
            }
        }
        return true;
    }

    bool isRoot() const{
        return _isRoot;
    }

    /**
     * * For double track
     * * Check that it is possible to insert if have same unit pointer
     * ! Not a good function
    */
    bool isPossibleInsert(uint64_t unit_offset, void *wait_insert_pointer){
        for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
            if(_tracks[unit_offset]._data[i].getBitmap(0) && _tracks[unit_offset]._data[i].getSize() < _options.unit_size && _tracks[unit_offset]._data[i].getPtr() == wait_insert_pointer){
                return true;
            }
        }

        return false;
    }

    void setLeaf(bool isLeaf){
        for(int i = 0; i < _options.unit_size; ++i){
            _tracks[i]._isLeaf = isLeaf;
        }
    }

    uint64_t getDataSize(uint64_t offset) const{
        return _tracks[offset].getSize();
    }

    uint64_t getValidSize() const{
        uint64_t counter = 0;
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

    Size sparse(){
        if(isLeaf()){
            return _options.unit_size - getValidSize();
        }
        Size counter = 0;
        for(int i = 0; i < _options.dataSize(isLeaf()); ++i){
            for(int j = 0; j < _options.unit_size; ++j){
                if(_tracks[j]._data[i].getBitmap(0)){
                    counter += ((Unit *)_tracks[j]._data[i].getPtr())->sparse();
                }
            }
        }
        for(int j = 0; j < _options.unit_size; ++j){
            if(hasFrontSideUnit(j)){
                counter += getFrontSideUnit(j)->sparse();
            }
        }

        return counter + _options.unit_size - getValidSize();
    }

    /* Data Member */

    Node *_tracks;
    
    /* System */

    void setRoot(bool status){
        _isRoot = status;
    }

    void sideChecker(){

    }

    bool _isRoot;
    Options _options;
    uint64_t _id;
};

#include<unordered_map>
namespace System{
    // store unit pointer
    // store how many the tracks of the unit is using
    //std::unordered_map<Unit *, uint64_t> unitPool;
    
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
            //unitPool[newUnit] = 1;
        }
        
        return newUnit;
    }
}

std::ostream &operator<<(std::ostream &out, const Unit::Node &right){
    //std::clog << "<log> Node Print" << std::endl;
    // std::clog << sizeof(right) << std::endl;

    //* status
    #ifdef RELEASE
    out << right._id << ",";
    out << right._shiftCounter << ",";
    out << right._insertCounter << ",";
    out << right._removeCounter << ",";
    out << right._readCounter << ",";
    out << right._migrateCounter;
    if(InstructionCounter % 1000000 == 0){
        out << std::endl;
    }
    else{
        out << "\n";
    }
    //* debug
    #elif defined DEBUG
    out << "\n\t\t(\n";
    // out << "\t\t\t" << "Node "<< right._id << "\n";
    // out << "\t\t\t" << right._shiftCounter << "\n";
    // out << "\t\t\t" << right._insertCounter << "\n";
    // out << "\t\t\t" << right._removeCounter << "\n";
    // out << "\t\t\t" << right._readCounter << "\n";
    // out << "\t\t\t" << right._migrateCounter << "\n";

    out << "\t\t\t";

    out << " _sideFront: " << right._sideFront;
    if(!right._sideFrontBitmap) out << "*";

    out << " ";

    bool first = true;
    for(int i = 0; i < right._options.dataSize(right.isLeaf()); ++i){
        if(first)first = false;
        else out << ", ";
        out << right._data[i];
    }
    
    out << " _sideBack: " << right._sideBack;
    if(!right._sideBackBitmap) out << "*";

    out << "\n\t\t)";

    if(!right.isValid()) out << "*";
    #else
        out << "not set output flag";
    #endif

    return out;
}

std::ostream &operator<<(std::ostream &out, const Unit &right){
    //std::clog << "<log> Unit " << right._id << " Print" << std::endl;

    #ifdef RELEASE
    // out << "Unit " << right._id << "\n";
    for(int i = 0; i < right._options.unit_size; ++i){
        out << right._tracks[i];
    }
    if(!right.isLeaf()){
        for(int i = 0; i < right._options.unit_size; ++i){
            if(right._tracks[i]._isValid && right._tracks[i]._sideFrontBitmap && right._tracks[i]._sideFront != nullptr){
                out << *(Unit *)right._tracks[i]._sideFront;
            }

            for(int j = 0; j < right._options.dataSize(right.isLeaf()); ++j){
                if(right._tracks[i]._isValid && right._tracks[i]._data[j].getBitmap(0) && right._tracks[i]._data[j].getPtr() != nullptr){ //TODO
                    out << *(Unit *)right._tracks[i]._data[j].getPtr();
                }
            }

            if(right._tracks[i]._isValid && right._tracks[i]._sideBackBitmap && right._tracks[i]._sideBack != nullptr){
                out << *(Unit *)right._tracks[i]._sideBack;
            }
        }
    }
    
    #elif defined DEBUG
    //* debug
    out << "\t[\t\n";
    out << "\t\t";
    if(right.isLeaf()) out << "Leaf Unit";
    else out << "Internal Unit";
    out << " " << right._id << " " << &right << "\n";
    if(right._options.unit_size > 1){
        out << "\t\t" << "Left Parent: " << right.getParentUnit(0) << " Offset " << right.getParentOffset(0)  << "\n";
        out << "\t\t" << "Right Parent: " << right.getParentUnit(1) << " Offset " << right.getParentOffset(1)  << "\n";
    }
    else{
        out << "\t\t" << "Parent: " << right.getParentUnit() << " Offset " << right.getParentOffset()  << "\n";
    }
    out << "\t\t";
    if(right._isRoot) out << "Root" << "\n";
    
    bool first = true;
    for(int i = 0; i < right._options.unit_size; ++i){
        if(first)first = false;
        else out << ", \n";
        out << right._tracks[i];
    }
    out << "\n\t]\n";
    if(!right.isLeaf()){
        for(int i = 0; i < right._options.unit_size; ++i){
            if(right._tracks[i]._isValid && right._tracks[i]._sideFrontBitmap && right._tracks[i]._sideFront != nullptr){
                out << *(Unit *)right._tracks[i]._sideFront;
            }

            for(int j = 0; j < right._options.dataSize(right.isLeaf()); ++j){
                if(right._tracks[i]._isValid && right._tracks[i]._data[j].getBitmap(0) && right._tracks[i]._data[j].getPtr() != nullptr){
                    out << *(Unit *)right._tracks[i]._data[j].getPtr();
                }
            }

            if(right._tracks[i]._isValid && right._tracks[i]._sideBackBitmap && right._tracks[i]._sideBack != nullptr){
                out << *(Unit *)right._tracks[i]._sideBack;
            }
        }
    }
    #else
        out << "not set output flag";
    #endif

    return out;
}

#endif