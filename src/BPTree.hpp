#ifndef BPTREE_H
#define BPTREE_H

#include <iostream>
#include "Options.hpp"
#include "Unit.hpp"

class BPTree{
    friend std::ostream &operator<<(std::ostream &out, const BPTree &right);
public:
    BPTree(Options options) : _options(options){
        _root = nullptr;
    }

    uint64_t *searchData(const Index &idx){
        Offset unit_offset = 0;
        return _root->searchData(idx, unit_offset);
    }

    void updateData(Index idx, uint64_t data){
        uint64_t unit_offset = 0;
        uint64_t *dataPtr = _root->searchData(idx, unit_offset);
        
        if(dataPtr != nullptr){
            *dataPtr = data;
        }
    }

    void insertData(const Index &idx, uint64_t data){
        if(_root == nullptr){
            _root = new Unit(_options);
            _leftMostUnit = _root;
            _root->_tracks[0].setValid(true);
        }
        
        _root->insertData(idx, data, 0, -1);
        
        if(_root->getParentUnit() != nullptr){
            _root = _root->getParentUnit();
        }
            
    }

    void deleteData(const Index &idx){
        if(_root == nullptr){
            return;
        }

        bool mergeFlag = false;
        _root->deleteData(idx, 0, -1, mergeFlag);

        if(!_root->_isRoot){
            _root = _root->getBackSideUnit(0);
        }

        if(_root != nullptr){
            _root->adjInternalIndex();
        }
    }

    void sideChecker(){

    }
    
private:
    Unit *_root;
    Unit *_leftMostUnit;

    /* System */
    Options _options;
};

std::ostream &operator<<(std::ostream &out, const BPTree &right){
    //std::clog << "<log> BPTree Print" << std::endl;
    //std::clog << "<log> &right: " << &right << std::endl;
    
    if(right._root == nullptr){
        out << "B+ Tree is not constructed!";
    }
    else{
        out << "{\n";
        out << *(right._root);
        out << "\n}\n";
    }
    
    return out;
}

#endif