#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
using namespace std;

#define RELEASE
// #define DEBUG

#define dbg(info) std::clog << info << std::endl;

#include "Options.hpp"
#include "BPTree.hpp"

enum Operation{
    END,
    SEARCH,
    INSERT,
    DELETE,
    UPDATE,
    SKIP,
    STOP,
    CHECK
};

int inputParser(const string &str, Index &idx, Data &data){
    stringstream ss(str);
    string op;
    ss >> op;
    if(op[0] == '#'){
        return Operation::SKIP;
    }
    ss >> idx >> data;
    if(data == 0) data = rand() % 26 + 'a';

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
        // return Operation::SKIP;
        return Operation::CHECK;
    }
    else if(op == ""){
        return Operation::SKIP;
    }

}

/**
 * * setting file
*/
Options settingParser(ifstream &fin){
    Size WORD_LENGTH = 0;
    Size TRACK_LENGTH = 0; 
    Size UNIT_SIZE = 0;
    Size KP_LENGTH = 0;
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
        #ifdef __linux__
        getline(ss, val, '\r');
        #elif _WIN64
        getline(ss, val);
        #endif
        
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
            else if(val == "RANGE_READ"){
                read_function = Options::read_function::RANGE_READ;
            }
        }
        else if(op == "search_function"){
            if(val == "TRAD_BINARY_SEARCH"){
                search_function = Options::search_function::TRAD_BINARY_SEARCH;
            }
            else if(val == "BIT_BINARY_SEARCH"){
                search_function = Options::search_function::BIT_BINARY_SEARCH;
            }
        }
        else if(op == "update_function"){
            if(val == "OVERWRITE"){
                update_function = Options::update_function::OVERWRITE;
            }
            else if(val == "PERMUTATION_WRITE"){
                update_function = Options::update_function::PERMUTATION_WRITE;
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

        std::clog << "<log> setting: " << op << " " << val << std::endl;
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
    #ifdef __linux__
        cout << "Using Linux" << endl;
    #elif _WIN64
        cout << "Using Windows 64" << endl;
    #else
        cout << "Not Support this operating system" << endl;
        reutrn EXIT_FAILURE;
    #endif

    // argument
	//* argument1 = data filename
    //* argument2 = setting filename
    //* argument3 = output filename
    //* argument4 = csv node information filename
    //* argument5 = csv tree information filename
    if(argc != 6){
        cout << "invalid arguments" << endl;
        cout << "arg1 = data, arg2 = setting, arg3 = status, arg4 = node csv, arg5 = tree csv" << endl;
        return EXIT_FAILURE;
    }
        
    //* load data
    ifstream workload(argv[1]);
    if(workload.fail()){
        cerr << "file: '" << argv[1] << "' open error" << endl;
        return EXIT_FAILURE;
    }

    //* load setting
    ifstream setting(argv[2]);
    if(setting.fail()){
        cerr << "file: '" << argv[2] << "' open error" << endl;
        return EXIT_FAILURE;
    }

    //* other message output
    ofstream fout(argv[3]);
    if(fout.fail()){
        cerr << "file: '" << argv[3] << "' open error" << endl;
        return EXIT_FAILURE;
    }

    //* csv node information
    ofstream fcsvNodeInfo(argv[4]);
    if(fcsvNodeInfo.fail()){
        cerr << "file: '" << argv[4] << "' open error" << endl;
        return EXIT_FAILURE;
    }

    //* csv tree information
    ofstream fcsvTreeHeight(argv[5]);
    if(fcsvTreeHeight.fail()){
        cerr << "file: '" << argv[5] << "' open error" << endl;
        return EXIT_FAILURE;
    }

    //* environmnet initial
    srand(0);
    clog << boolalpha;

    BPTree tree(settingParser(setting));
    string input;

    fcsvNodeInfo << "id,shiftCounter,insertCounter,removeCounter,readCounter,migrateCounter" << endl;

    fcsvTreeHeight << "inst_th,height" << endl;
    fcsvTreeHeight << 0 << ",";
    fcsvTreeHeight << 0 << std::endl;
    
    try{
        #ifdef __linux__
        while(getline(workload, input)){
            // workload.ignore();
        #elif _WIN64
        while(getline(workload, input)){
        #endif
            if(InstructionCounter % 1000000 == 0){
                clog << "Processing " << InstructionCounter << endl;
            }
            ++InstructionCounter;
            Index index = 0;
            Data data = 0;
            switch(inputParser(input, index, data)){
                case Operation::SEARCH:
                    try{
                        Data *dataPtr = tree.searchData(index);
                        
                        #ifdef DEBUG
                        if(dataPtr == nullptr)
                            fout << "Search index " << index << ": " << "Not found" << endl;
                        else
                            fout << "Search index " << index << ": " << *dataPtr << endl;
                        #endif
                    }
                    catch(const char* e){
                        cout << e << endl;
                    }
                    break;
                case Operation::INSERT:
                    ++InsertionCounter;
                    tree.insertData(index, data, fcsvTreeHeight);
                    #ifdef DEBUG
                    // fout << "Insert index " << index << ": " << data << endl;
                    fout << "insert " << index << endl;
                    // fout << tree << endl;
                    #endif
                    break;
                case Operation::DELETE:
                    tree.deleteData(index);
                    #ifdef DEBUG
                    fout << "Delete index " << index << endl;
                    #endif
                    break;
                case Operation::UPDATE:
                    tree.updateData(index, data);
                    #ifdef DEBUG
                    fout << "Update index " << index << ": " << data << endl;
                    #endif
                    break;
                case Operation::SKIP:
                    continue;
                case Operation::STOP:
                    throw "Debug force stop";
                case Operation::CHECK:
                    fout << tree << endl;
                    break;
                default:
                    throw "undefined BPTree operation";
            }
            // std::clog << "<log> <main()> " << input << " finish" << std::endl;
        }
        clog << "<log> input file: " << argv[1] << " success" << endl;

        fout << "Sparse Node: " << tree.sparse() << endl;

        fcsvTreeHeight << InstructionCounter << ",";
        fcsvTreeHeight << tree.height() << "\n";

        fcsvNodeInfo << tree << endl;
    }
    catch(const char *e){
        cout << e << endl;
        clog << e << endl;
        return EXIT_FAILURE;
    }

    workload.close();
    setting.close();
    fout.close();

    std::clog << "<log> exit success" << std::endl;

    return EXIT_SUCCESS;
}
