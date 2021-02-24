#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <algorithm>
#include <vector>

namespace System{
    unsigned binary_search(){
    
    }

    unsigned *makeSortedArray(unsigned *a, unsigned a_len, unsigned *b, unsigned b_len){
        unsigned t_len = a_len + b_len;
        std::vector<unsigned> V(t_len);

        int it = 0;
        for(int i = 0; i < a_len; ++i, ++it){
            V[it] = a[i];
        }
        for(int i = 0; i < b_len; ++i, ++it){
            V[it] = b[i];
        }

        std::sort(V.begin(), V.end());

        unsigned *arr = new unsigned[t_len];
        for(int i = 0; i < t_len; ++i){
            arr[i] = V[i];
        }

        return arr;
    }

    unsigned *makeSplitDeleteIndexesArray(unsigned *a, unsigned a_len, unsigned mid){
        unsigned *arr = new unsigned[a_len / 2];
        int it = 0;

        for(int i = 0; i < a_len; ++i){
            if(a[i] >= mid){
                arr[it++] = i;
            }
        }

        return arr;
    }
}

#endif