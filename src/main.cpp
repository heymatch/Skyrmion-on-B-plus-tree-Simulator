#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
using namespace std;

#include "Options.hpp"
#include "BPTree.hpp"

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
        const unsigned UNIT_SIZE = 2; 

        Options options_traditional(
            WORD_LENGTH,
            TRACK_LENGTH,
            UNIT_SIZE,
            1,
            Options::ordering::UNSORTED,
            Options::read_function::SEQUENTIAL,
            Options::search_function::SEQUENTIAL,
            Options::update_function::OVERWRITE,
            Options::insert_function::SEQUENTIAL,
            Options::delete_function::SEQUENTIAL,
            Options::split_merge_function::TRAD
        );
        
        try{
            BPTree tree(options_traditional);
            unsigned data;
            while(fin >> data){
                tree.insertData(data);
            }
            cout << tree << endl;
        }
        catch(const char *e){
            cout << e << endl;
        }
        
    }
    
    return EXIT_SUCCESS;
}
