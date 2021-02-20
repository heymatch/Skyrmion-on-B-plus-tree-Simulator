#ifndef OPTIONS_H
#define OPTIONS_H

struct Options{
    unsigned word_length;
    unsigned track_length; // number of words
    unsigned unit_size; // e.g., single track, double tracks

    unsigned kp_length; // access port sharing

    enum struct ordering : unsigned{
        None,
        UNSORTED,
        SORTED
    };
    ordering node_ordering;
    void checkSorted(){
        if(node_ordering != ordering::SORTED)
            throw "This operation needs sorted structure";
    }

    enum struct read_function : unsigned{
        None,
        SEQUENTIAL, 
        RANGE_READ
    };
    read_function read_mode;

    enum struct search_function : unsigned{
        None,
        SEQUENTIAL,
        TRAD_BINARY_SEARCH,
        BIT_BINARY_SEARCH
    };
    search_function search_mode;

    enum struct update_function : unsigned{
        None,
        OVERWRITE,
        PERMUTATION_WRITE,
        PERMUTE_WORD_COUNTER,
        PERMUTE_FEW_COUNTER
    };
    update_function update_mode;

    enum struct insert_function : unsigned{
        None,
        SEQUENTIAL,
        BIT_BINARY_INSERT
    };
    insert_function insert_mode;

    enum struct delete_function : unsigned{
        None,
        SEQUENTIAL,
        BALANCE
    };
    delete_function delete_mode;

    enum struct split_merge_function : unsigned{
        None,
        TRAD,
        UNIT
    };
    split_merge_function split_merge_mode;
    

    Options(
            unsigned word_length = 0,
            unsigned track_length = 0,
            unsigned unit_size = 0,
            unsigned kp_length = 0,
            ordering node_ordering = ordering::None,
            read_function read_mode = read_function::None,
            search_function search_mode = search_function::None,
            update_function update_mode = update_function::None,
            insert_function insert_mode = insert_function::None,
            delete_function delete_mode = delete_function::None,
            split_merge_function split_merge_mode = split_merge_function::None
        ) :
        word_length(word_length),
        track_length(track_length),
        unit_size(unit_size),
        kp_length(kp_length),
        node_ordering(node_ordering),
        read_mode(read_mode),
        search_mode(search_mode),
        update_mode(update_mode),
        insert_mode(insert_mode),
        delete_mode(delete_mode),
        split_merge_mode(split_merge_mode)
    {
        
    }
};


#endif