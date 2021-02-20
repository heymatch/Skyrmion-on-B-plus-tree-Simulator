#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
using namespace std;

#include "Options.hpp"
#include "Node.hpp"

unsigned UnitId = 0;
unsigned LeafNodeId = 0;
unsigned InternalNodeId = 0;

unsigned ShiftOP = 0;
unsigned InjectOP = 0;
unsigned DetectOP = 0;
unsigned MigrateOP = 0;

int main(int argv, char **argc){
    // argument
	// argument1 = dataname
    if(argv != 2)
        return EXIT_FAILURE;
    
    // load data
    ifstream fin(argc[1]);

    // traditional operation on single track skyrmion
    {
        const unsigned WORD_LENGTH = 32;
        const unsigned TRACK_LENGTH = 8;
        const unsigned UNIT_SIZE = 1; 

        Options options_traditional;
        options_traditional.word_length = WORD_LENGTH;
        options_traditional.track_length = TRACK_LENGTH;
        options_traditional.unit_size = UNIT_SIZE;
        options_traditional.kp_length = 1;
        options_traditional.node_ordering = Options::ordering::UNSORTED;
        options_traditional.read_mode = Options::read_function::SEQUENTIAL;
        options_traditional.search_mode = Options::search_function::SEQUENTIAL;
        options_traditional.update_mode = Options::update_function::OVERWRITE;
        options_traditional.insert_mode = Options::insert_function::SEQUENTIAL;
        options_traditional.delete_mode = Options::delete_function::SEQUENTIAL;
        options_traditional.split_merge_mode = Options::split_merge_function::TRAD;
        
        try{
            Node tNode(options_traditional);
            unsigned data;
            while(fin >> data){
                tNode.insertData(data);
            }
            cout << tNode << endl;
        }
        catch(const char *e){
            cout << e << endl;
        }
        
    }
    
    return EXIT_SUCCESS;
}
