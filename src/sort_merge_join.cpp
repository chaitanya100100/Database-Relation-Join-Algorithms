#include<bits/stdc++.h>
using namespace std;


void write_buffer(vector<pair<string, string> > & buf, string fpath)
{
    ofstream ofd(fpath);
    if(!ofd.good())
    {
        cerr << "Error in opening " << fpath << endl;
        exit(-1);
    }
    for(int i = 0; i < buf.size(); i++)
        ofd << buf[i].second << "\n";
    ofd.close();
}


class SortMergeJoin;
class RelationSort
{
private:
    int T; // number of tuples of relation
    int B; // number of blocks of relation
    int TB; // number of tuples in a block
    int MM; // number of main memory blocks
    int keycol_num;
    string path;
    string name;
    ifstream fd;
    vector<string> sublist_paths;
    vector<ifstream> subl_fds;
    vector< vector<pair<string, string> > > loaded_subl;

public:
    // SortMergeJoin can access private members
    friend class SortMergeJoin;

    // count number of tuples and blocks
    void init(string path, int TB, int MM, int keycol_num)
    {
        this->path = path;
        this->TB = TB;
        this->MM = MM;
        this->keycol_num = keycol_num;

        int off = path.find_last_of("/");
        if(off == string::npos) this->name = path;
        else this->name = path.substr(off+1, path.length() - off);

        fd.open(path);
        if(!fd.good())
        {
            cerr << "Error in opening " << path << endl;
            exit(-1);
        }
        T = count(istreambuf_iterator<char>(fd), istreambuf_iterator<char>(), '\n');
        fd.close();

        B = (T + TB - 1) / TB;
    }

    // create sorted sublists
    void create_sorted_sublists()
    {
        fd.open(path);
        if(!fd.good())
        {
            cerr << "Error in opening " << path << endl;
            exit(-1);
        }

        sublist_paths.clear();
        subl_fds.clear();

        string line, key;
        int idx = 0, offset;
        vector<pair<string, string> > outbuf;

        while(getline(fd, line))
        {
            // construct record with this line
            offset = line.find_first_of(" ");
            if(keycol_num == 0)
                key = line.substr(0, offset);
            else
                key = line.substr(offset+1, line.length()-offset);

            outbuf.push_back({key, line});
            if(outbuf.size() == TB*MM)
            {
                sort(outbuf.begin(), outbuf.end());
                string opp = INTER_PATH + "/" + name + "_" + to_string(idx) + ".txt";
                sublist_paths.push_back(opp);
                write_buffer(outbuf, opp);
                idx++;
                outbuf.clear();
            }
        }
        if(outbuf.size())
        {
            sort(outbuf.begin(), outbuf.end());
            string opp = INTER_PATH + "/" + name + "_" + to_string(idx) + ".txt";
            sublist_paths.push_back(opp);
            write_buffer(outbuf, opp);
            idx++;
            outbuf.clear();
        }
    }

    void init_rel_get_next()
    {
        loaded_subl = vector<vector<pair<string, string> > >(sublist_paths.size());
        for(int i = 0; i < sublist_paths.size(); i++)
        {
            subl_fds.push_back(ifstream(sublist_paths[i]));
            if(!subl_fds[i].good())
            {
                cerr << "Error in opening " << sublist_paths[i] << endl;
                exit(-1);
            }
        }
    }

    pair<string, string> rel_get_next(int idx)
    {
        if(loaded_subl[idx].size() == 0)
        {
            string line, key;
            int offset;
            while(getline(subl_fds[idx], line))
            {
                offset = line.find_first_of(" ");
                if(keycol_num == 0)
                    key = line.substr(0, offset);
                else
                    key = line.substr(offset+1, line.length()-offset);

                loaded_subl[idx].push_back({key, line});
                if(loaded_subl[idx].size() >= TB) break;
            }
            reverse(loaded_subl[idx].begin(), loaded_subl[idx].end());
        }
        if(loaded_subl[idx].size() == 0) return {"", ""};
        pair<string, string> ret = loaded_subl[idx].back();
        loaded_subl[idx].pop_back();
        return ret;
    }

    void rel_close()
    {
        fd.close();
        for(int i = 0; i < subl_fds.size(); i++)
        {
            if(subl_fds[i].is_open())
                subl_fds[i].close();
            // remove(sublist_paths[i].c_str());
        }
    }
};

struct Tuple
{
    string tup;
    int rel;
    int subl_idx;

    Tuple(){};

    Tuple(string & _tup, int _rel, int _subl_idx):
    tup(_tup),
    rel(_rel),
    subl_idx(_subl_idx)
    {}
};

class SortMergeJoin
{
private:
    int MM; // number of main memory blocks
    int TB; // number of tuples in a block
    string R_path;
    string S_path;
    RelationSort R, S;
    multimap<string, Tuple> mp;
    ofstream out_fd;
    string out_path;

public:

    SortMergeJoin(string R_path, string S_path, int MM, int TB)
    {
        this->MM = MM;
        this->TB = TB;
        this->R_path = R_path;
        this->S_path = S_path;
        cout << "MM : " << MM << endl;
        cout << "TB : " << TB << endl;

        join_open();

        init_join_gen_next();

        while(join_gen_next());

        join_close();

        cout << "done" << endl;
    }

    void join_open()
    {
        R.init(R_path, TB, MM, 1);
        S.init(S_path, TB, MM, 0);
        cout << "R.T : " << R.T << endl;
        cout << "R.B : " << R.B << endl;
        cout << "S.T : " << S.T << endl;
        cout << "S.B : " << S.B << endl;

        if(R.B + S.B >= (MM-1)*MM)
        {
            cerr << "M is too small for given input relations R and S" << endl;
            exit(-1);
        }

        R.create_sorted_sublists();
        S.create_sorted_sublists();

        out_path = "./" + R.name + "_" + S.name + "_join.txt";
        out_path = "out_sort.txt";
        cout << out_path << endl;
        out_fd.open(out_path);
    }

    void init_join_gen_next()
    {
        R.init_rel_get_next();
        S.init_rel_get_next();

        mp.clear();
        pair<string, string> tup;
        Tuple x;
        for(int i = 0; i < R.subl_fds.size(); i++)
        {
            for(int j = 0; j < TB; j++)
            {
                tup = R.rel_get_next(i);
                if(tup.first.empty()) break;
                x = Tuple(tup.second, 0, i);
                mp.insert({tup.first, x});
            }
        }
        for(int i = 0; i < S.subl_fds.size(); i++)
        {
            for(int j = 0; j < TB; j++)
            {
                tup = S.rel_get_next(i);
                if(tup.first.empty()) break;
                x = Tuple(tup.second, 1, i);
                mp.insert({tup.first, x});
            }
        }
    }

    int join_gen_next()
    {
        // cout << "Current map" << endl;
        // for(auto it = mp.begin(); it != mp.end(); it++)
        // {
        //     cout << "key : " << it->first << " | tup : " << (it->second).tup << " | rel : " << (it->second).rel <<
        //     " | " << (it->second).subl_idx << endl;
        // }
        if(mp.empty())
            return 0;

        pair<string, Tuple> small = *mp.begin();
        vector<Tuple> R_part, S_part;
        int oldR = 0, oldS = 0;

        while(1){
            if(mp.empty()) break;
            if(mp.begin()->first != small.first) break;

            auto to = mp.upper_bound(small.first);
            for(auto it = mp.begin(); it != to; it++)
            {
                if((it->second).rel == 0) R_part.push_back(it->second);
                else S_part.push_back(it->second);
            }
            mp.erase(mp.begin(), to);

            int s_idx;
            pair<string, string> tup;
            Tuple x;
            for(int i = oldR; i < R_part.size(); i++)
            {
                s_idx = R_part[i].subl_idx;
                tup = R.rel_get_next(s_idx);
                if(tup.first.empty()) continue;
                x = Tuple(tup.second, 0, s_idx);
                mp.insert({tup.first, x});
            }
            for(int i = oldS; i < S_part.size(); i++)
            {
                s_idx = S_part[i].subl_idx;
                tup = S.rel_get_next(s_idx);
                if(tup.first.empty()) continue;
                x = Tuple(tup.second, 1, s_idx);
                mp.insert({tup.first, x});
            }
            oldR = R_part.size();
            oldS = S_part.size();
        }

        string op, fop;
        int off, en;
        for(int i = 0; i < R_part.size(); i++)
            for(int j = 0; j < S_part.size(); j++)
            {
                op = R_part[i].tup + " " + S_part[j].tup + "\n";

                off = op.find_first_of(" ");
                en = op.find_first_of(" ", off+1);
                fop = op.substr(0, off) + op.substr(en);
                out_fd << fop;
            }

        return 1;
    }

    void join_close()
    {
        R.rel_close();
        S.rel_close();
        out_fd.close();
    }
};
