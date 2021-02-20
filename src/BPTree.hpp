#ifndef BPTREE_H
#define BPTREE_H

#include "Unit.hpp"

class BPTree{
    friend std::ostream &operator<<(std::ostream &out, const BPTree &right);
public:
    BPTree(Options options) : _options(options){
        _root = nullptr;
    }

    void searchDate(){

    }

    void updateData(){

    }

    void insertData(unsigned data){
        if(_root == nullptr){
            _root = new Unit(_options);
        }
        _root->insertData(data, 0);
    }

    void deleteDate(){

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