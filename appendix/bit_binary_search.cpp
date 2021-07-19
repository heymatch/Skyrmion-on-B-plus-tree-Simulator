#include <bits/stdc++.h>
using namespace std;

bool bit_binary_search(uint64_t data[], uint64_t searchKey){
    uint64_t lower = 0, upper = 1024, middle = (lower + upper) / 2;
    uint64_t check = 1ull << 63;
    // cout << check << endl;
    for(int bit_offset = 0; bit_offset < 64; ++bit_offset){
        
        // cout << middle << endl;
        // cout << data[middle] << endl;
        // cout << ((searchKey << bit_offset) & check) << endl;
        // clog << "upper: " << upper << endl;
        // clog << "lower: " << lower << endl;
        
        if(((data[middle] << bit_offset) & check) == ((searchKey << bit_offset) & check)){
             
        }
        else{
            if(((searchKey << bit_offset) & check) == 0){
                upper = middle - 1;
            }
            else if(((searchKey << bit_offset) & check) == check){
                lower = middle + 1;
            }
            middle = (lower + upper) / 2;
        }
        
    }

    return data[middle] == searchKey;
}

int main(){
    uint64_t data[1024] = {};
    uint64_t data_temp[1024] = {};
    uint64_t searchKey;

    srand(time(0));
    for(int i = 0; i < 1024; ++i){
        data[i] = rand() % 32;
    }
    sort(data, data+32);

    try{
        for(int i = 0; i < 32; ++i){
            // if(bit_binary_search(data, data[i])){
            //     cout << "i=" << i << ": " << "find " << data[i] << "\n";
            // }
            // else{
            //     cout << "i=" << i << ": " << "not find " << data[i] << "\n";
            // }
            bitset<64> bit(data[i]);
            cout << bit.to_string() << "\n";
        }
    }
    catch(const char* e){
        cout << e << endl;
    }

    return 0;
}