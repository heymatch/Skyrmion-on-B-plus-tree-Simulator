#ifndef OPTIONS_H
#define OPTIONS_H

struct Options{
    unsigned word_length;
    unsigned track_length; // number of words
    unsigned unit_size; // e.g., single track, double tracks

    unsigned kp_length; // access port sharing

    unsigned node_structure;
    enum node_ordering{
        UNSORTED,
        SORTED
    };

    unsigned read_mode;
    enum read_function{
        TRAD, 
        RANGE_READ
    };

    unsigned search_mode;
    enum search_function{
        TRAD,
        BIT_BINARY_SEARCH
    };

    unsigned update_mode;
    enum update_function{
        TRAD, // overwrite
        PERMUTATION_WRITE,
        PERMUTE
    };

    unsigned insert_mode;
    enum insert_function{
        TRAD, // scan insert
        BIT_BINARY_INSERT
    };

    unsigned delete_mode;
    enum delete_function{
        TRAD,
        BALANCE
    };

    unsigned split_merge_mode;
    enum split_merge_mode{
        TRAD,
        UNIT
    };
};


#endif