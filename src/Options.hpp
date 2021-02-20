#ifndef OPTIONS_H
#define OPTIONS_H

struct Options{
    unsigned word_length;
    unsigned track_length; // number of words
    unsigned unit_size; // e.g., single track, double tracks

    unsigned kp_length; // access port sharing

    unsigned node_ordering;
    enum ordering{
        UNSORTED,
        SORTED
    };
    void checkSorted(){
        if(node_ordering != ordering::SORTED)
            throw "This operation needs sorted structure";
    }

    unsigned read_mode;
    enum read_function{
        SEQUENTIAL, 
        RANGE_READ
    };

    unsigned search_mode;
    enum search_function{
        SEQUENTIAL,
        TRAD_BINARY_SEARCH,
        BIT_BINARY_SEARCH
    };

    unsigned update_mode;
    enum update_function{
        OVERWRITE,
        PERMUTATION_WRITE,
        PERMUTE_WORD_COUNTER,
        PERMUTE_FEW_COUNTER
    };

    unsigned insert_mode;
    enum insert_function{
        SEQUENTIAL,
        BIT_BINARY_INSERT
    };

    unsigned delete_mode;
    enum delete_function{
        SEQUENTIAL,
        BALANCE
    };

    unsigned split_merge_mode;
    enum split_merge_mode{
        TRAD,
        UNIT
    };

    
};


#endif