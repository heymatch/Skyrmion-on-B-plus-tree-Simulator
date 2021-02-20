#ifndef UNIT_H
#define UNIT_H

#include "Options.hpp"
#include "Node.hpp"

struct Unit{
    Unit(){

    }

    void readNode(){
        switch (_options.read_mode)
        {
        case Options::read_function::SEQUENTIAL:
            /* code */
            break;
        case Options::read_function::RANGE_READ:
            /* code */
            break;
        default:
            break;
        }
    }

    void updateNode(){
        switch (_options.update_mode)
        {
        case Options::update_function::OVERWRITE:
            /* code */
            break;
        case Options::update_function::PERMUTATION_WRITE:
            /* code */
            break;
        case Options::update_function::PERMUTE_WORD_COUNTER:
            /* code */
            break;
        case Options::update_function::PERMUTE_FEW_COUNTER:
            /* code */
            break;
        default:
            throw "undefined update operation";
            break;
        }
    }

    void insertNode(){
        switch (_options.insert_mode)
        {
        case Options::insert_function::SEQUENTIAL:
            switch (_options.node_ordering)
            {
            case Options::ordering::SORTED:
                /* code */
                break;
            case Options::ordering::UNSORTED:
                /* code */
                break;
            default:
                throw "undefined operation";
                break;
            }
            break;
        case Options::insert_function::BIT_BINARY_INSERT:
            /* code */
            throw "Developing";
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    void deleteNode(){
        switch (_options.delete_mode)
        {
        case Options::delete_function::SEQUENTIAL:
            /* code */
            break;
        case Options::delete_function::BALANCE:
            /* code */
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    void splitNode(){
        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
            /* code */
            break;
        case Options::split_merge_function::UNIT:
            /* code */
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    void mergeNode(){
        switch (_options.split_merge_mode)
        {
        case Options::split_merge_function::TRAD:
            /* code */
            break;
        case Options::split_merge_function::UNIT:
            /* code */
            break;
        default:
            throw "undefined operation";
            break;
        }
    }

    //Node *_tracks;
    ////
    Options _options;
};

#endif