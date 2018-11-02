#include<bits/stdc++.h>
using namespace std;

// number of tuples in one block
int TB = 100;
// folder to store intermediate files
string INTER_PATH = "../files/inter_files";

#include "sort_merge_join.cpp"
#include "hash_join.cpp"

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

    if(type == "sort")
        SortMergeJoin(R_path, S_path, M, TB);
    else if(type == "hash")
        HashJoin(R_path, S_path, M, TB);
    else
        cerr << "type should be sort or hash" << endl;
    return 0;
}
