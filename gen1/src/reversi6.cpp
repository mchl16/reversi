#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <poll.h>
#include <boost/unordered_map.hpp>
using namespace std;

//Do zrobienia: 
//-wydzielić obliczenia (MCTS) do osobnego procesu
//-ten proces uwielowątkowić

constexpr int DEPTH=2;
constexpr int DIFF=DEPTH;
constexpr int COUNT=120;
constexpr int CPUCOUNT=4;

int ACTUAL_COUNT=0;

constexpr long long MODULO=5342931457063200LL;

struct kekdatabase{
    typedef pair<size_t,size_t> data_t;
    boost::unordered_map<data_t,long long> data; //boost::

    bool f(size_t& b){
        size_t B=(b&(b>>1))&0x5555555555555555LL;
        int res=__builtin_ctz(b);
        if(res!=64){
            b^=(3<<(2*res));
            return true;
        }
        else return false;
    }

    data_t cut(const char *s){
        size_t buf[8];
        memcpy(buf,s,64);
        for(int i=1;i<4;++i){
            buf[0]+=buf[2*i]<<(2*i);
            buf[1]+=buf[2*i+1]<<(2*i);
        }

        if(!f(buf[0])) f(buf[1]);
        return {buf[0],buf[1]};
    }

    long long& operator[](char *s){
        return data[cut(s)];
    }
}; 

kekdatabase database,db[4];
// unordered_map<string,long long> database;

constexpr int gen[8][2]={{1,0},{0,1},{-1,0},{0,-1},{1,1},{-1,1},{-1,-1},{1,-1}};

struct rng_gen{
    random_device rd;
    mt19937_64 generator;
    uniform_int_distribution<long long> distribution;

    rng_gen() : generator(mt19937_64(rd())),distribution(uniform_int_distribution<long long>(0,MODULO-1)){}

    long long get(){
        return distribution(generator);
    }
} rng[CPUCOUNT];

sigjmp_buf env;
void sigalrm_handler(int s){
    cerr << "Nie dali mi :C" << endl;
    siglongjmp(env,1);
}

struct reversi{
    char board[66];

    reversi(){
        memset(board,3,64);
        board[27]=2;
        board[28]=1;
        board[35]=1;
        board[36]=2;
        board[64]=1;
        board[65]=0;
    }

    static bool in_bounds(int x,int y){
        if(x<0) return false;
        if(x>=8) return false;
        if(y<0) return false;
        return y<8;
    }

    void place_piece(int field){
        assert(board[field]==3);
        assert(field<64);
        board[field]=board[64];
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-board[64]);g1+=i[0],g2+=i[1]);
            if(!in_bounds(g1,g2) || board[8*g1+g2]!=board[64]) continue;
            for(g1=f1+i[0],g2=f2+i[1];in_bounds(g1,g2) && board[8*g1+g2]==3-board[64];g1+=i[0],g2+=i[1]){
                assert(8*g1+g2!=64);
                board[8*g1+g2]=board[64];
            }
        }
    }

    bool check_move(int field) const{
        if(field<0) return true;
        if(board[field]!=3) return false;
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-board[64]);g1+=i[0],g2+=i[1]);
            if(in_bounds(g1,g2) && board[8*g1+g2]==board[64]){
                if(g1!=f1+i[0] || g2!=f2+i[1]) return true;
            }
        }
        return false;
    }

    void move(int field){
        if(field>=0) place_piece(field);
        board[64]=3-board[64];
    }

    void print(){
        cerr << "Wg reversi5 (ruch " << (board[64]==1 ? "o" : "#") << "):" << endl;
        for(int i=0;i<8;++i){
            for(int j=0;j<8;++j){
                const char *c=".o#.";
                cerr << c[board[8*i+j]];
            }
            cerr << "\n";
        }
        cerr << endl;
    }
};

int mcts(reversi& pos,kekdatabase& db,int n,bool passed=false){
    int res=0;
    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(passed){
            constexpr int T2[]={0,1,-1,0};
            res=0;
            for(int i=0;i<64;++i) res+=T2[pos.board[i]];
            if(res>0) res=1;
            else if(res<0) res=-1;

            db[pos.board]+=res;
            return res;
        }
        reversi pos2=pos;
        pos2.move(-1);
        res=mcts(pos2,db,n,true);

        db[pos.board]+=res;
        return res;
    }
        
    reversi pos2=pos;
    pos2.move(t[rng[n].get()%j]);
    res=mcts(pos2,db,n);

    db[pos.board]+=res;
    return res;
}

void *mcts_thread(void *arg){
    long long* N=(long long*)arg;
    int n=N[1];
    reversi **pos=(reversi**)(arg);
    // *pos->print();
    for(int i=0;i<ACTUAL_COUNT;++i){
        reversi pos2=**pos;
        mcts(pos2,db[n],n);
    }
    pthread_exit(nullptr);
}

void mcts_driver(reversi& pos){
    long long res=0;
    pthread_t tids[CPUCOUNT];
    void *args[2*CPUCOUNT];
    for(int i=0;i<CPUCOUNT;++i){
        new (&rng[i]) rng_gen();
        args[2*i]=&pos;
        args[2*i+1]=nullptr;
        memset(&args[2*i+1],i,1);
    }

    for(int i=0;i<CPUCOUNT;++i){
        assert(pthread_create(&tids[i],nullptr,mcts_thread,&args[2*i])==0);
    }
    for(int i=0;i<CPUCOUNT;++i) pthread_join(tids[i],nullptr);

    for(int i=0;i<CPUCOUNT;++i){
        for(auto it=db[i].data.begin();it!=db[i].data.end();++it) database.data[it->first]+=it->second;
        db[i].data.clear();
    }
}

int prealphabeta(reversi &pos,int depth=DEPTH,bool passed=false){
    if(depth==0) return 1;

    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(passed) return 0;

        reversi pos2=pos;
        pos2.move(-1);
        return prealphabeta(pos2,depth-1,true);
    }

    reversi T[64];
    for(int i=0;i<j;++i){
        T[t[i]]=pos;
        T[t[i]].move(t[i]);
    }

    int res=0;
    for(int i=0;i<j;++i) res+=prealphabeta(T[t[i]],depth-1);
    return res;
}

pair<long long,int> alphabeta(reversi& pos,long long a=-1e18,long long b=1e18,int depth=DEPTH,int last=-1,bool passed=false){
    bool black=(pos.board[64]==1);
    long long val=0;
    int res=last;
    if(depth==0){
        mcts_driver(pos);
        return {database[pos.board],res};
    } 

    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(passed){
            constexpr int T2[]={0,1,-1,0};
            long long res2=0;
            for(int i=0;i<64;++i) res2+=T2[pos.board[i]];
            if(res2>0) res2=1;
            else if(res2<0) res2=-1;
            return {1e18*res2,-1};
        }
        if(depth==DEPTH) return {database[pos.board],-1};
        else{
            pos.move(-1);
            return alphabeta(pos,a,b,depth-1,-1,true);
        }
    }

    reversi T[64];
    long long ranks[64];
    for(int i=0;i<j;++i){
        T[t[i]]=pos;
        T[t[i]].move(t[i]);
        ranks[t[i]]=database[T[t[i]].board];
    }
    
    if(black) sort(t,t+j,[&ranks](int a,int b){return ranks[a]>ranks[b];});
    else sort(t,t+j,[&ranks](int a,int b){return ranks[a]<ranks[b];});

    for(int i=0;i<j;++i){
        auto ab=alphabeta(T[t[i]],a,b,depth-1,t[i],false);

        if(black){
            if(ab.first>a){
                a=ab.first;
                val=a;
                res=t[i];
            }
        }
        else{
            if(ab.first<b){
                b=ab.first;
                val=b;
                res=t[i];
            }
        }
        if(a>b) break;
    }
    return {val,res};
}

void make_move(reversi& pos,int usecs,int cnt){
    int c=prealphabeta(pos);
    if(c==0){
        cout << "IDO -1 -1" << endl;
        pos.move(-1);
        return;
    }
    
    ACTUAL_COUNT=1+COUNT*1000/(c*(65-cnt));
    itimerval T;
    memset(&T,0,sizeof(T));
    T.it_value.tv_usec=usecs%1000000;
    T.it_value.tv_sec=usecs/1000000;
    
    int x=-1;
    assert(setitimer(ITIMER_REAL,&T,nullptr)>=0);
    
    if(sigsetjmp(env,1)){
        cerr << "Nie dali mi :C" << endl;
        bool black=pos.board[64]==1;
        long long res=1e18*(black ? -1 : 1);
        for(int i=0;i<64;++i) {
            if(!pos.check_move(i)) continue;
            reversi pos2=pos;
            pos2.move(i);
            long long d=database[pos2.board];
            if(black && d>res){
                res=d;
                x=i;
            }
            else if(!black && d<res){
                res=d;
                x=i;
            }
        }
    }
    else x=alphabeta(pos).second;

    memset(&T,0,sizeof(T));
    setitimer(ITIMER_REAL,&T,nullptr);
    cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << endl;
    pos.move(x);
}

int main(){
    string s;
    reversi g;
    int cnt=1;
    
    // import_data();
    // exit(0);

    // struct sigaction sa;
    // sa.sa_handler=sigterm_handler;
    // sigaction(SIGTERM,&sa,NULL);

    struct sigaction sa2;
    sa2.sa_handler=sigalrm_handler;
    sigaction(SIGALRM,&sa2,NULL);

    cout << "RDY" << endl;
    for(;;){
        cin >> s;
        if(s=="HEDID"){
            double a,b;
            int c,d;
            cin >> a >> b >> c >> d;
            // cerr << b << "!" << endl;
            g.move(8*d+c);
            // g.print();
            // assert(g.board[64]==3-cnt);
            make_move(g,min(900000*a,500000*b),cnt);
            cnt+=2;
            // g.print();
            // assert(g.board[64]==cnt);
        }
        else if(s=="ONEMORE"){
            g=reversi();
            cnt=0;
            cout << "RDY" << endl;
        }
        else if(s=="BYE"){
            exit(0);
        }
        else if(s=="UGO"){
            cnt=0;
            make_move(g,4000000,cnt);
            cnt+=2;
            // g.print();
            // assert(g.board[64]==cnt);
        }
    }

    // on_exit();
}