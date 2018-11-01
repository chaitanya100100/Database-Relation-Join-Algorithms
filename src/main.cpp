#include<bits/stdc++.h>
using namespace std;

#include "sort_merge_join.cpp"

// number of tuples in one block
int TB = 10;

int main(int argc, char **argv)
{
    if(argc != 5)
    {
        cerr << "Usage : " << argv[0] << " <R_path> <S_path> <sort/hash> <M>" << endl;
        exit(-1);
    }
    string R_path(argv[1]);
    string S_path(argv[2]);
    string type(argv[3]);
    int M = atoi(argv[4]);

    ifstream R_fd(R_path);
    ifstream S_fd(S_path);
    if(!R_fd.good() || !R_fd.good() || M <= 0 || (type != "hash" && type != "sort"))
    {
        cerr << "Invalid input arguments" << endl;
        cerr << "Usage : " << argv[0] << " <R_path> <S_path> <sort/hash> <M>" << endl;
        R_fd.close();
        S_fd.close();
        exit(-1);
    }
    R_fd.close();
    S_fd.close();

    SortMergeJoin(R_path, S_path, M, TB);

    return 0;
}
