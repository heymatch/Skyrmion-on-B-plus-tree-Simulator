#include <bits/stdc++.h>
using namespace std;

int main(int argc, char **argv){
    if(argc != 3)
        return EXIT_FAILURE;

    ofstream fout("testcase.txt");
    
    srand(atoi(argv[2]));
    int numberOfTestcases = atoi(argv[1]);
    int base = atoi(argv[1]);

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
        fout << "delete " << index[i] << endl;
    }

    fout.close();

    return 0;
}