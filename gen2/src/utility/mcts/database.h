#ifndef DATABASE_H
#define DATABASE_H

#include <cstring>
#include <utility>
#include <boost/unordered_map.hpp>


#include "../basegame/reversi.h"
#include "../threads/threads.h"

using namespace std;

struct frac{
    int num;
    unsigned int den;

    bool operator<(frac other){
        return ((long long)num)*other.den<((long long)other.num)*den;
    }

    bool operator>(frac other){
        return ((long long)num)*other.den>((long long)other.num)*den;
    }

    bool operator<=(frac other){
        return ((long long)num)*other.den<=((long long)other.num)*den;
    }

    bool operator>=(frac other){
        return ((long long)num)*other.den>=((long long)other.num)*den;
    }

    void operator=(const frac& other){
        memcpy(this,&other,8);
    }

    void operator+=(const frac& s){
        this->num+=s.num;
        this->den+=s.den;
    }

    frac() : num(0),den(1){}

    frac(int a,int b) : num(a),den(b){}
};

frac fast_sigmoid(frac f);

double fast_sigmoid(double d);

struct database{
    typedef pair<size_t,size_t> data_t;
    boost::unordered_map<data_t,frac> data;

    struct query_t{
        const char *s;
        int cnt;
    };

    static bool f(size_t& b){
        size_t B=(b&(b>>1))&0x5555555555555555LL;
        int res=__builtin_ctz(b);
        if(res!=64){
            b^=(3<<(2*res));
            return true;
        }
        else return false;
    }

    static data_t cut(const char *s){
        auto c=(s[64]==2);

        size_t buf[8];
        memcpy(buf,s,64);
        for(int i=0;i<4;++i){ //can this be done faster?
            buf[0]+=buf[2*i]<<(2*i);
            buf[1]+=buf[2*i+1]<<(2*i);
        }

        if(c && !f(buf[0])) f(buf[1]); //encode "white to move"
        return {buf[0],buf[1]};
    }

    frac& operator[](const char *s){
        return data[cut(s)];
    }
};

extern database db;

struct tree_database{
    typedef pair<size_t,size_t> key_t;

    struct data_t{
        frac val;
        bool visited;
        bool explored;

        struct move_t{
            int n; //number of move
            int curr; //threads currently exploring this move
            data_t *stats; //pointer to stats of move

            move_t(int n,data_t *stats) : n(n),curr(0),stats(stats){} 
        };
        vector<move_t> data;

        data_t(frac f) : val(f),visited(false),explored(false){} 

        data_t() : data_t(frac(0,0)){}

        void add_move(int n,data_t *val){
            data.push_back(move_t(n,val));
        }

        void change_threads(int n,int val){
            for(int i=0;;++i) if(data[i].n==n){
                data[i].curr+=val;
                break;
            }
        }

        void set_threads(int n,int val){
            for(int i=0;;++i) if(data[i].n==n){
                data[i].curr=val;
                break;
            }
        }

        int pick(bool white){ 
            double uct=-2137;
            int res=-1;

            for(int i=0;i<data.size();++i){
                if(data[i].stats->explored) continue;
                
                double k=(double)data[i].stats->val.num/data[i].stats->val.den;
                if(white) k=-k;
                k+=2.0*fast_sigmoid(-data[i].stats->val.den);
                if(k>uct){
                    uct=k;
                    res=data[i].n;
                }
            }
            return res;
        }

        bool all_explored(){
            for(int i=0;i<data.size();++i){
                if(data[i].stats->explored) data[i].curr=-1;
                if(data[i].curr!=-1) return false;
            }
            explored=true;
            return true;
        }
    };

    boost::unordered_map<key_t,data_t> data;

    static bool f(size_t& b){
        size_t B=(b&(b>>1))&0x5555555555555555LL;
        int res=__builtin_ctz(b);
        if(res!=64){
            b^=(3<<(2*res));
            return true;
        }
        else return false;
    }

    struct hashtable{
        char t[256];

        static constexpr int f(int a,int b,int c,int d){ //a,b,c,d in {3,1,2}
            return a+4*b+16*c+64*d;
        }

        static constexpr int g(int a,int b,int c,int d){ //a,b,c,d in {0,1,2}
            return a+3*b+9*c+27*d;
        }

        static constexpr void h(int n){
            int a=n%3;
            n/=3;
            int b=n%3;
            n/=3;
            int c=n%3;
            n/=3;
            int d=n%3;
            t[f(3-a,3-b,3-c,3-d)]=g(a,b,c,d);
        }
        constexpr hashtable(){
            for(int i=0;i<81;++i) h(i);
        }
    };

    static constexpr hashtable ht{};

    static key_t hash_pos(const reversi& pos){
        size_t buf[8];
        memcpy(buf,&pos.board,64);
        for(int i=0;i<4;++i){ //can this be done faster?
            buf[0]+=buf[2*i]<<(2*i);
            buf[1]+=buf[2*i+1]<<(2*i);
        }

        char *t=(char*)buf;
        for(int i=0;i<2;++i) t[i]=ht[t[i]];
      
        if(pos.turn==2) t[0]+=128; //encode "white to move"
        if(pos.passed) t[1]+=128; //encode "passed"
        return {buf[0],buf[1]};
    }

    bool contains(const reversi& pos){
        return data.find(hash_pos(pos))!=data.end();
    }

    data_t& operator[](const reversi& pos){
        auto c=hash_pos(pos);
        auto it=data.find(c);
        if(it!=data.end()) return it->second;
        auto it2=db.data.find(c);
        data[c]=data_t(it2!=db.data.end() ? it2->second : frac());
        return data[c];
    }

};
extern tree_database game_db;

int mcts_driver(reversi& pos,int usecs);

#endif