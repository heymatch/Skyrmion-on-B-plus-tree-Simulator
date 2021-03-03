#include <bits/stdc++.h>
using namespace std;

int main(){
    ofstream fout("testcase.txt");
    
    int numberOfTestcases = 100;
    int base = 100;

    int index[numberOfTestcases] = {};
    for(int i = 1; i <= numberOfTestcases; ++i){
        index[i] = i + base;
    }

    for(int i = 1; i <= numberOfTestcases; ++i){
        swap(index[i], index[rand() % numberOfTestcases + 1]);
    }

    for(int i = 1; i <= numberOfTestcases; ++i){
        fout << "insert " << index[i] << endl;
    }

    for(int i = 1; i <= numberOfTestcases; ++i){
        fout << "search " << index[i] << endl;
    }

    fout.close();

    return 0;
}