#ifndef OPTIONS_H
#define OPTIONS_H

struct Options{
    unsigned word_length;
    unsigned track_length; // number of words
    unsigned unit_size;

    unsigned kp_length;

    unsigned read_mode;
    unsigned update_mode;
    unsigned insert_mode;
    unsigned delete_mode;

    enum function_list{
        TRD_READ,
        TRD_UPDATE,
        TRD_INSERT,
        TRD_DELETE,
        TRD_SPLIT,
        TRD_MERGE,
        TRD_BORROW,
        
        PERMUTATION_WRITE,
        
        RANGE_READ,
        PERMUTE_UPDATE,
        PERMUTE_INSERT,
        MARK_DELETE,
        UNIT_MERGE,
        UNIT_SPLIT,
        UNIT_BALANCE
    };
};


#endif