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

    unsigned *searchData(unsigned idx){
        return _root->searchData(idx, 0);
    }

    void updateData(unsigned idx, unsigned data){
        unsigned *dataPtr = _root->searchData(idx, 0);
        if(dataPtr != nullptr){
            *dataPtr = data;
        }
    }

    void insertData(unsigned idx, unsigned data){
        if(_root == nullptr){
            _root = new Unit(_options);
        }

        _root->insertData(idx, data, 0);
        
        if(_root->getParentUnit() != nullptr){
            _root = _root->getParentUnit();
        }
            
    }

    void deleteData(unsigned idx){
        if(_root == nullptr){
            return;
        }

        bool mergeFlag = false;
        _root->deleteData(idx, 0, -1, mergeFlag);

        if(!_root->_isRoot){
            _root = _root->getSideUnit();
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
        out << "{\n";
        out << *(right._root);
        out << "\n}\n";
    }
    
    return out;
}

#endif