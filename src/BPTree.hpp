#ifndef BPTREE_H
#define BPTREE_H

#include "System.hpp"
#include "Unit.hpp"
#include <iostream>

class BPTree{
    friend std::ostream &operator<<(std::ostream &out, const BPTree &right);
public:
    BPTree(Options options) : _options(options){
        _root = nullptr;
    }

    unsigned *searchData(unsigned idx){
        return _root->searchData(idx);
    }

    void updateData(unsigned idx, unsigned data){
        unsigned *dataPtr = _root->searchData(idx);
        if(dataPtr != nullptr){
            *dataPtr = data;
        }
    }

    void insertData(unsigned idx, unsigned data){
        if(_root == nullptr){
            _root = new Unit(_options);
        }
        _root->insertData(idx, data, 0);
        
        if(_root->_tracks[0]._parent != nullptr){
            _root = _root->_tracks[0]._parent;
            //std::clog << "<log> _root: " << _root->_tracks[0] << std::endl;
            //std::clog << "<log> _root->_tracks[0]._parent: " << _root->_tracks[0]._parent << std::endl;
        }
            
    }

    void deleteData(unsigned idx){
        bool mergeFlag = false;
        _root->deleteData(idx, 0, -1, mergeFlag);
        if(!_root->_isRoot){
            _root = _root->getSideUnit();
        }
        if(_root != nullptr){
            _root->adjInternalIndex();
        }
    }
    /*
    Node *findParent(Unit *child){
        
    }*/
    
private:
    Unit *_root;
    ////
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