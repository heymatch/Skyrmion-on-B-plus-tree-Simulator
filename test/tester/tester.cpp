#include <bits/stdc++.h>
using namespace std;

int main(int argc, char **argv){
    if(argc < 3)
        return EXIT_FAILURE;

    ofstream fout("testcase.txt");
    
    int type = atoi(argv[1]);
    int numberOfTestcases = atoi(argv[2]);
    int base = atoi(argv[2]);
    if(argc == 4)
        srand(atoi(argv[3]));

    if(type == 1){
        cout << "generate insert and search dataset" << endl;
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
            swap(index[i], index[rand() % numberOfTestcases + 1]);
        }

        for(int i = 1; i <= numberOfTestcases; ++i){
            fout << "search " << index[i] << endl;
        }
    }
    else if(type == 2){
        cout << "generate insert and delete dataset" << endl;
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
            swap(index[i], index[rand() % numberOfTestcases + 1]);
        }

        for(int i = 1; i <= numberOfTestcases; ++i){
            fout << "delete " << index[i] << endl;
        }
    }
    else if(type == 3){
        cout << "generate insert, delete and search mixed dataset" << endl;
        set<int> S;
        int index[numberOfTestcases] = {};
        for(int i = 1; i <= numberOfTestcases; ++i){
            index[i] = i + base;
        }

        for(int i = 1; i <= numberOfTestcases; ++i){
            swap(index[i], index[rand() % numberOfTestcases + 1]);
        }

        for(int i = 1; i <= numberOfTestcases; ++i){
            int r = rand() % 3;
            int idx = rand() % numberOfTestcases + 1;
            if(r == 0){
                fout << "insert " << index[idx] << endl;
                S.insert(index[idx]);
            }
            else if(r == 1){
                if(S.count(index[idx])){
                    fout << "delete " << index[idx] << endl;
                    S.erase(index[idx]);
                }
                else{
                    --i;
                }
            }
            else if(r == 2){
                if(S.count(index[idx])){
                    fout << "search " << index[idx] << endl;
                }
                else{
                    --i;
                }
            }
        }
    }
    else{
        cout << "Not supported type" << endl;
    }

    fout.close();

    return 0;
}