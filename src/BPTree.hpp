#ifndef BPTREE_H
#define BPTREE_H

#include "Unit.hpp"

class BPTree{
    friend std::ostream &operator<<(std::ostream &out, const BPTree &right);
public:
    BPTree(Options options) : _options(options){
        _root = nullptr;
    }

    void searchData(unsigned idx){

    }

    void updateData(){

    }

    void insertData(unsigned idx, unsigned data){
        if(_root == nullptr){
            _root = new Unit(_options);
        }
        if(_root->isFull(0)){
            splitNode();
        }
        _root->insertData(idx, data, 0);
    }

    void deleteData(){

    }

    void splitNode(){

    }

    void mergeNode(){

    }
    
private:
    Unit *_root;
    ////
    Options _options;
};

std::ostream &operator<<(std::ostream &out, const BPTree &right){
    out << "{\n";
    out << *(right._root);
    out << "\n}\n";
    return out;
}

#endif