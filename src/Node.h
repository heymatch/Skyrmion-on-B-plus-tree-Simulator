/**
 * 
 * 
 **/

#ifndef NODE_H
#define NODE_H

#include "options.h"

struct KeyPtrSet{

};

struct Node{
    Node(Options options){
        _options = options;
    }

    void readTraditional(){

    }

    void updateTraditional(){

    }

    void insertTraditional(){

    }

    void deleteTraditional(){

    }

    void readRangeRead(){

    }

    KeyPtrSet *_data;
    ////
    Options _options;
};

#endif