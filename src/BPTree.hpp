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

    Data *searchData(const Index &idx){
        Offset unit_offset = 0;
        return _root->searchData(idx, unit_offset);
    }

    void updateData(const Index &idx, const Data &data){
        Offset unit_offset = 0;
        Data *dataPtr = _root->searchData(idx, unit_offset);
        
        if(dataPtr != nullptr){
            *dataPtr = data;
        }
    }

    void insertData(const Index &idx, const Data &data, std::ostream &fcsvTreeHeight){
        if(_root == nullptr){
            _root = new Unit(_options);
            _leftMostUnit = _root;
            _root->_tracks[0].setValid(true);

            fcsvTreeHeight << InstructionCounter << ",";
            fcsvTreeHeight << height() << "\n";
        }

        _root->insertData(idx, data, 0, -1);
        
        if(_root->getParentUnit() != nullptr){
            _root = _root->getParentUnit();

            fcsvTreeHeight << InstructionCounter << ",";
            fcsvTreeHeight << height() << "\n";
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

    Size height(){
        if(_root == nullptr) return 0;

        Unit *current = _root;
        Size counter = 1;
        while(!current->isLeaf()){
            counter += 1;
            current = current->getFrontSideUnit(0);
        }

        return counter;
    }

    Size sparse(){
        if(_root == nullptr) return 0;

        return _root->sparse();
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
        #ifdef RELEASE
        
        out << *(right._root);
        #elif defined DEBUG
        out << "{\n";
        out << *(right._root);
        out << "\n}\n";
        #else 
        out << "not set output flag";
        #endif
    }
    
    return out;
}

#endif