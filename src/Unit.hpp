#ifndef UNIT_H
#define UNIT_H

#include "Node.hpp"

struct Unit{
    Unit(){

    }

    void readData(){
        switch (_options.read_mode)
        {
        case Options::read_function::TRAD:
            /* code */
            break;
        
        default:
            break;
        }
    }

    void updateDate(){

    }

    void insertData(){

    }

    void deleteData(){

    }

    void split(){

    }

    void merge(){

    }

    //Node *_tracks;
    ////
    Options _options;
};

#endif