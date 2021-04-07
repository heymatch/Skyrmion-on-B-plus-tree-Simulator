#ifndef BPTREE_H
#define BPTREE_H

#include "Unit.hpp"
#include <iostream>

class BPTree{
    friend std::ostream &operator<<(std::ostream &out, const BPTree &right);
public:
    BPTree(Options options) : _options(options){
        _root = nullptr;
    }

    uint64_t *searchData(uint64_t idx){
        uint64_t unit_offset = 0;
        return _root->searchData(idx, unit_offset);
    }

    void updateData(uint64_t idx, uint64_t data){
        uint64_t unit_offset = 0;
        uint64_t *dataPtr = _root->searchData(idx, unit_offset);
        
        if(dataPtr != nullptr){
            *dataPtr = data;
        }
    }

    void insertData(uint64_t idx, uint64_t data){
        if(_root == nullptr){
            _root = new Unit(_options);
            _root->_tracks[0].setValid(true);
        }
        
        _root->insertData(idx, data, 0, -1);
        
        if(_root->getParentUnit() != nullptr){
            _root = _root->getParentUnit();
        }
            
    }

    void deleteData(uint64_t idx){
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

    ////Node *findParent(Unit *child);
    
private:
    Unit *_root;

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
        //out << "{\n";
        out << *(right._root);
        //out << "\n}\n";
    }
    
    return out;
}

#endif