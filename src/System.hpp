#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <algorithm>
#include <vector>
#include <ostream>
#include "KeyPtrSet.hpp"

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
/*
    KeyPtrSet *makeSortedArray(KeyPtrSet *arr, unsigned len){
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
    }*/

    unsigned getMid(KeyPtrSet *arr, unsigned len, unsigned insert){
        unsigned t_len = (arr[0]._capacity - 1) * len + 1;
        std::vector<unsigned> V(t_len);

        int it = 0;
        for(int i = 0; i < len; ++i){
            for(int j = 0; j < arr[i]._capacity-1; ++j, ++it){
                V[it] = arr[i].getKey(j);
                //std::clog << V[it] << " ";
            }
        }
        V[it++] = insert;
        //std::clog << V[it-1] << " ";
        //std::clog << std::endl;
        //std::clog << it << std::endl;
        //std::clog << V[t_len / 2] << std::endl;

        std::sort(V.begin(), V.end());

        return V[t_len / 2];
    }

    unsigned *makeSplitDeleteIndexesArray(unsigned &out_len, unsigned *a, unsigned a_len, unsigned mid){
        unsigned *arr = new unsigned[a_len / 2];
        int it = 0;

        for(int i = 0; i < a_len; ++i){
            
            if(a[i] >= mid){
                arr[it++] = i;
                //std::clog << arr[it-1] << " ";
            }
            
        }
        //std::clog << std::endl;

        out_len = it;
        return arr;
    }
}

#endif