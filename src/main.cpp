#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
using namespace std;

#include "Options.hpp"
#include "BPTree.hpp"

enum Operation{
    SEARCH,
    INSERT,
    DELETE,
    UPDATE,
    SKIP
};

unsigned parser(string str, unsigned &idx, unsigned &data){
    stringstream ss(str);
    string op;
    ss >> op >> idx >> data;
    for(char &it: op){
        it = tolower(it);
    }
    if(op == "search"){
        return Operation::SEARCH;
    }
    else if(op == "insert"){
        return Operation::INSERT;
    }
    else if(op == "delete"){
        return Operation::DELETE;
    }
    else if(op == "update"){
        return Operation::UPDATE;
    }
    else if(op == ""){
        return Operation::SKIP;
    }
    else if(op[0] == '#'){
        return Operation::SKIP;
    }
}

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
        const unsigned TRACK_LENGTH = 4;
        const unsigned UNIT_SIZE = 1;
        const unsigned KP_LENGTH = 2;

        Options options_traditional(
            WORD_LENGTH,
            TRACK_LENGTH,
            UNIT_SIZE,
            KP_LENGTH,
            Options::ordering::SORTED,
            Options::read_function::SEQUENTIAL,
            Options::search_function::SEQUENTIAL,
            Options::update_function::OVERWRITE,
            Options::insert_function::SEQUENTIAL,
            Options::delete_function::SEQUENTIAL,
            Options::split_merge_function::TRAD
        );
        
        BPTree tree(options_traditional);
        string input;
        try{
            while(getline(fin, input)){
                unsigned index = 0, data = 0;
                switch(parser(input, index, data)){
                    case Operation::SEARCH:
                        try{
                            tree.searchData(index);
                        }
                        catch(const char* e){
                            cout << e << endl;
                        }
                        break;
                    case Operation::INSERT:
                        tree.insertData(index, data);
                        break;
                    case Operation::DELETE:
                        break;
                    case Operation::UPDATE:
                        break;
                    case Operation::SKIP:
                        break;
                    default:
                        throw "undefined BPTree operation";
                }
            }
            cout << tree << endl;
        }
        catch(const char *e){
            cout << e << endl;
        }
    }

    std::clog << "<log> exit success" << std::endl;
    return EXIT_SUCCESS;
}
