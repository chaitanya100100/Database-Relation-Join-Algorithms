#include<bits/stdc++.h>
using namespace std;


hash<string> hasher;

class HashJoin;

class RelationHash
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

    int NBK; // number of buckets
    vector<string> bucket_paths;
    vector<ofstream> bucket_ofds;
    vector<ifstream> bucket_ifds;
    vector<vector<pair<string, string> > > bucket_bufs;
    vector<int> bucket_size;

public:
    friend class HashJoin;

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

    // hash each tuple to a bucket
    void create_hashed_buckets(int num_bucks)
    {
        NBK = num_bucks;

        bucket_paths.clear();
        bucket_ofds.clear();
        bucket_ifds.clear();
        bucket_bufs.clear();

        bucket_paths.resize(NBK);
        bucket_ofds.resize(NBK);
        bucket_ifds.resize(NBK);
        bucket_bufs.resize(NBK);
        bucket_size.resize(NBK);

        for(int i = 0; i < NBK; i++)
        {
            bucket_paths[i] = INTER_PATH + "/" + name + "_" + to_string(i) + ".txt";
            bucket_ofds[i].open(bucket_paths[i]);
            bucket_bufs[i].clear();
            bucket_size[i] = 0;
            if(!bucket_ofds[i].good())
            {
                cerr << "Error in opening " << bucket_paths[i] << endl;
                exit(-1);
            }
        }

        fd.open(path);
        if(!fd.good())
        {
            cerr << "Error in opening " << path << endl;
            exit(-1);
        }

        string line, key;
        int bid, offset;

        while(getline(fd, line))
        {
            // construct record with this line
            offset = line.find_first_of(" ");
            if(keycol_num == 0)
                key = line.substr(0, offset);
            else
                key = line.substr(offset+1, line.length()-offset);

            bid = hasher(key) % NBK;
            bucket_bufs[bid].push_back({key, line});

            if(bucket_bufs[bid].size() == TB)
                write_buffer(bid);
        }
        for(int bid = 0; bid < NBK; bid++)
            if(bucket_bufs[bid].size())
                write_buffer(bid);

        for(int i = 0; i < NBK; i++)
            if(bucket_ofds[i].is_open())
                bucket_ofds[i].close();
    }

    void write_buffer(int bid)
    {
        if(!bucket_ofds[bid].good())
        {
            cerr << "Error writing to buffer " << bucket_paths[bid] << endl;
            exit(-1);
        }

        for(int i = 0; i < bucket_bufs[bid].size(); i++)
            bucket_ofds[bid] << bucket_bufs[bid][i].second << "\n";

        // bucket_size[bid] += bucket_bufs[bid].size();
        bucket_size[bid] += 1;
        bucket_bufs[bid].clear();
    }

    void init_rel_get_next(int bid)
    {
        bucket_ifds[bid].open(bucket_paths[bid]);
        bucket_bufs[bid].clear();
    }

    pair<string, string> rel_get_next(int bid)
    {
        if(bucket_bufs[bid].size() == 0)
        {
            string line, key;
            int offset;
            while(getline(bucket_ifds[bid], line))
            {
                // construct record with this line
                offset = line.find_first_of(" ");
                if(keycol_num == 0)
                    key = line.substr(0, offset);
                else
                    key = line.substr(offset+1, line.length()-offset);
                assert(hasher(key) % NBK == bid);
                bucket_bufs[bid].push_back({key, line});
                if(bucket_bufs[bid].size() >= TB) break;
            }
        }
        if(bucket_bufs[bid].size() == 0) return {"", ""};
        pair<string, string> ret = bucket_bufs[bid].back();
        bucket_bufs[bid].pop_back();
        return ret;
    }

    void rel_close()
    {
        fd.close();
        for(int i = 0; i < bucket_ofds.size(); i++)
        {
            if(bucket_ofds[i].is_open())
                bucket_ofds[i].close();
            // remove(bucket_paths[i].c_str());
        }
    }
};


class HashJoin
{
private:
    int MM; // number of main memory blocks
    int TB; // number of tuples in a block
    int NBK ; // number of buckets
    string R_path;
    string S_path;
    RelationHash R, S;
    ofstream out_fd;
    string out_path;

public:
    HashJoin(string R_path, string S_path, int MM, int TB)
    {
        this->MM = MM;
        this->TB = TB;
        this->R_path = R_path;
        this->S_path = S_path;
        cout << "MM : " << MM << endl;
        cout << "TB : " << TB << endl;

        NBK = MM;
        hash_join_open();

        // this is equaivalent to gen_next() function
        for(int i = 0; i < NBK; i++)
            hash_join_gen(i);

        hash_join_close();

        cout << "done" << endl;
    }

    void hash_join_open()
    {
        R.init(R_path, TB, MM, 1);
        S.init(S_path, TB, MM, 0);
        cout << "R.T : " << R.T << endl;
        cout << "R.B : " << R.B << endl;
        cout << "S.T : " << S.T << endl;
        cout << "S.B : " << S.B << endl;

        if(min(R.B, S.B) >= (MM-1)*MM)
        {
            cerr << "M is too small for given input relations R and S" << endl;
            exit(-1);
        }

        R.create_hashed_buckets(NBK);
        S.create_hashed_buckets(NBK);

        out_path = "./" + R.name + "_" + S.name + "_join.txt";
        out_path = "out_hash.txt";
        cout << out_path << endl;
        out_fd.open(out_path);
    }

    void hash_join_gen(int bid)
    {
        int R_sz = R.bucket_size[bid];
        int S_sz = S.bucket_size[bid];
        int is_R_small = (R_sz < S_sz);
        RelationHash & small = (is_R_small) ? R : S;
        RelationHash & big = (!is_R_small) ? R : S;

        if(small.bucket_size[bid] >= MM)
        {
            cerr << "bucket " << bid << " of small relation " << small.name << " can't fit into memroy" << endl;
            cerr << "continuing the code" << endl;
        }

        // cout << small.name << " " << small.bucket_size[bid] << " " << big.name << " " << big.bucket_size[bid] << endl;

        multimap<string, string> mp;
        small.init_rel_get_next(bid);
        big.init_rel_get_next(bid);

        pair<string, string> tup;
        while(1)
        {
            tup = small.rel_get_next(bid);
            if(tup.first.empty()) break;
            mp.insert({tup.first, tup.second});
        }

        string op, fop;
        int off, en;
        
        while(1)
        {
            tup = big.rel_get_next(bid);
            if(tup.first.empty()) break;

            auto range = mp.equal_range(tup.first);
            for(auto it = range.first; it != range.second; it++)
            {
                if(is_R_small)
                    op = it->second + " " + tup.second + "\n";
                else
                    op = tup.second + " " + it->second + "\n";

                off = op.find_first_of(" ");
                en = op.find_first_of(" ", off+1);
                fop = op.substr(0, off) + op.substr(en);
                out_fd << fop;
            }
        }
    }

    void hash_join_close()
    {
        R.rel_close();
        S.rel_close();
        out_fd.close();
    }
};
