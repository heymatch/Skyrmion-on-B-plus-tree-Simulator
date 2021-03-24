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
    SKIP,
    STOP,
    CHECK
};

unsigned inputParser(string str, unsigned &idx, unsigned &data){
    stringstream ss(str);
    string op;
    ss >> op;
    if(op[0] == '#'){
        return Operation::SKIP;
    }
    ss >> idx >> data;
    if(data == 0)data = rand() % 1000 + 1;

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
    else if(op == "!!!"){
        return Operation::STOP;
    }
    else if(op == "@@@"){
        return Operation::CHECK;
    }
    else if(op == ""){
        return Operation::SKIP;
    }
}

/**
 * TODO setting file
*/
Options settingParser(ifstream &fin){
    unsigned WORD_LENGTH = 0;
    unsigned TRACK_LENGTH = 0; 
    unsigned UNIT_SIZE = 0;
    unsigned KP_LENGTH = 0;
    Options::ordering ordering = Options::ordering::None;
    Options::read_function read_function = Options::read_function::None;
    Options::search_function search_function = Options::search_function::None;
    Options::update_function update_function = Options::update_function::None;
    Options::insert_function insert_function = Options::insert_function::None;
    Options::delete_function delete_function = Options::delete_function::None;
    Options::split_merge_function split_merge_function = Options::split_merge_function::None;

    string input;
    while(getline(fin, input)){
        stringstream ss(input);
        string op, val;
        getline(ss, op, '=');
        getline(ss, val);
        
        if(op == "word_length"){
            WORD_LENGTH = stoi(val);
        }
        else if(op == "track_length"){
            TRACK_LENGTH = stoi(val);
        }
        else if(op == "unit_size"){
            UNIT_SIZE = stoi(val);
        }
        else if(op == "kp_length"){
            KP_LENGTH = stoi(val);
        }
        else if(op == "ordering"){
            if(val == "SORTED"){
                ordering = Options::ordering::SORTED;
            }
        }
        else if(op == "read_function"){
            if(val == "SEQUENTIAL"){
                read_function = Options::read_function::SEQUENTIAL;
            }
        }
        else if(op == "search_function"){
            if(val == "SEQUENTIAL"){
                search_function = Options::search_function::SEQUENTIAL;
            }
        }
        else if(op == "update_function"){
            if(val == "OVERWRITE"){
                update_function = Options::update_function::OVERWRITE;
            }
        }
        else if(op == "insert_function"){
            if(val == "SEQUENTIAL"){
                insert_function = Options::insert_function::SEQUENTIAL;
            }
        }
        else if(op == "delete_function"){
            if(val == "SEQUENTIAL"){
                delete_function = Options::delete_function::SEQUENTIAL;
            }
        }
        else if(op == "split_merge_function"){
            if(val == "TRAD"){
                split_merge_function = Options::split_merge_function::TRAD;
            }
            else if(val == "UNIT"){
                split_merge_function = Options::split_merge_function::UNIT;
            }
        }
    }
    
    Options options(
        WORD_LENGTH,
        TRACK_LENGTH,
        UNIT_SIZE,
        KP_LENGTH,
        ordering,
        read_function,
        search_function,
        update_function,
        insert_function,
        delete_function,
        split_merge_function
    );

    return options;
}

int main(int argc, char **argv){
    // argument
	// * argument1 = data filename
    // * argument2 = setting filename
    if(argc != 3)
        return EXIT_FAILURE;
    
    // * load data
    ifstream workload(argv[1]);
    if(workload.fail()){
        cerr << "file: '" << argv[1] << "' open error" << endl;
        return EXIT_FAILURE;
    }

    // * load setting
    ifstream setting(argv[2]);
    if(setting.fail()){
        cerr << "file: '" << argv[2] << "' open error" << endl;
        return EXIT_FAILURE;
    }

    // initial
    srand(0);

    {
        BPTree tree(settingParser(setting));
        string input;
        
        try{
            while(getline(workload, input)){
                unsigned index = 0, data = 0;
                switch(inputParser(input, index, data)){
                    case Operation::SEARCH:
                        try{
                            unsigned *dataPtr = tree.searchData(index);
                            if(dataPtr == nullptr)
                                cout << "Search index " << index << ": " << "Not found" << endl;
                            else
                                cout << "Search index " << index << ": " << *dataPtr << endl;
                        }
                        catch(const char* e){
                            cout << e << endl;
                        }
                        break;
                    case Operation::INSERT:
                        tree.insertData(index, data);
                        cout << "Insert index " << index << ": " << data << endl;
                        break;
                    case Operation::DELETE:
                        tree.deleteData(index);
                        cout << "Delete index " << index << endl;
                        break;
                    case Operation::UPDATE:
                        tree.updateData(index, data);
                        cout << "Update index " << index << ": " << data << endl;
                        break;
                    case Operation::SKIP:
                        continue;
                    case Operation::STOP:
                        throw "Debug force stop";
                    case Operation::CHECK:
                        cout << tree << endl;
                        break;
                    default:
                        throw "undefined BPTree operation";
                }
                //std::clog << "<log main> " << input << " finish" << std::endl;
            }
            std::clog << "<log> input file: " << argv[1] << " success" << std::endl;
            cout << tree << endl;
        }
        catch(const char *e){
            cout << e << endl;
            clog << e << endl;
            return EXIT_FAILURE;
        }
    }

    std::clog << "<log> exit success" << std::endl;
    return EXIT_SUCCESS;
}
