#ifndef OPTIONS_H
#define OPTIONS_H

typedef uint64_t Index;
typedef size_t Offset;
typedef size_t Size;
typedef uint8_t Data;

struct Options{
    // e.g., 32, 64
    Size word_length;

    // number of words in a track
    //! odd number leads to problem
    // e.g., 4, 8, 16
    Size track_length;

    // e.g., 1 = single track, 2 = double tracks
    Size unit_size; 

    // key pointer length
    // at least 2
    // 1 pointer + n keys
    // e.g., 2, 3
    Size kp_length; 

    // access port sharing
    //? not use
    Size ap_sharing; 

    enum struct ordering : Size{
        None,
        UNSORTED, //! skip
        SORTED
    };
    ordering node_ordering;
    void checkSorted(){
        if(node_ordering != ordering::SORTED)
            throw "This operation needs sorted structure";
    }

    enum struct read_function : Size{
        None,
        SEQUENTIAL, 
        RANGE_READ
    };
    read_function read_mode;

    enum struct search_function : Size{
        None,
        SEQUENTIAL,
        TRAD_BINARY_SEARCH,
        BIT_BINARY_SEARCH
    };
    search_function search_mode;

    enum struct update_function : Size{
        None,
        OVERWRITE,
        PERMUTATION_WRITE,
        PERMUTE_WORD_COUNTER, //! skip
        PERMUTE_WITHOUT_COUNTER
    };
    update_function update_mode;

    enum struct insert_function : Size{
        None,
        SEQUENTIAL,
        BIT_BINARY_INSERT
    };
    insert_function insert_mode;

    enum struct delete_function : Size{
        None,
        SEQUENTIAL,
        BALANCE
    };
    delete_function delete_mode;

    enum struct split_merge_function : Size{
        None,
        TRAD,
        UNIT
    };
    split_merge_function split_merge_mode;
    

    Options(
            Size word_length = 0,
            Size track_length = 0,
            Size unit_size = 0,
            Size kp_length = 0,
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