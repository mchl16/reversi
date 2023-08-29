#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <poll.h>
#include <boost/unordered_map.hpp>
using namespace std;

//Do zrobienia: 
//alphabeta w koncowce, by je grac najlepiej

constexpr int DEPTH=1;
constexpr int DIFF=DEPTH;
constexpr int COUNT=500;
constexpr int CPUCOUNT=4;
constexpr int SAVE_DEPTH=40;

int cnt=0;
int shortdb[64];

int ACTUAL_COUNT=0;

constexpr long long MODULO=5342931457063200LL;

struct frac{
    unsigned int num,den;

    bool operator<(frac other){
        return ((long long)num)*other.den<((long long)other.num)*den;
    }

    bool operator>(frac other){
        return ((long long)num)*other.den>((long long)other.num)*den;
    }

    void operator=(const frac& other){
        memcpy(this,&other,8);
    }

    void operator+=(size_t s){
        *((size_t*)this)+=s;
        assert(num<=den);
    }

    void operator+=(const frac& s){
        *((size_t*)this)+=*((size_t*)&s);
        assert(num<=den);
    }
};

struct kekdatabase{
    typedef pair<size_t,size_t> data_t;
    boost::unordered_map<data_t,frac> data; //boost::

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

    frac& operator[](char *s){
        return data[cut(s)];
    }
}; 

kekdatabase database,db[4],db2[4];

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

pthread_t tids[CPUCOUNT];

sigjmp_buf env;
void sigalrm_handler(int s){
    cerr << "Nie dali mi :C" << endl;
    for(int i=0;i<4;++i){
        pthread_cancel(tids[i]);
        pthread_join(tids[i],nullptr);
    }
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
        board[field]=board[64];
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-board[64]);g1+=i[0],g2+=i[1]);
            if(!in_bounds(g1,g2) || board[8*g1+g2]!=board[64]) continue;
            for(g1=f1+i[0],g2=f2+i[1];in_bounds(g1,g2) && board[8*g1+g2]==3-board[64];g1+=i[0],g2+=i[1]){
                board[8*g1+g2]=board[64];
            }
        }
    }

    bool check_move(int field) const{
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
        cerr << "Wg reversi9 (ruch " << (board[64]==1 ? "o" : "#") << "):" << endl;
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

size_t mcts(reversi& pos,kekdatabase& db,kekdatabase& db2,int depth,bool passed=false){
    size_t res=2LL<<32;
    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(passed){
            constexpr int T2[]={0,1,-1,0};
            int x=0;
            for(int i=0;i<64;++i) x+=T2[pos.board[i]];
            
            
            if(x==0) ++res;
            else if(x>0) res+=2;

            //frac y=*(frac*)&res;

            if(depth<=SAVE_DEPTH) db[pos.board]+=res;
            else if(depth==cnt+1) db2[pos.board]+=res;
            return res;
        }
        reversi pos2=pos;
        pos2.move(-1);
        res=mcts(pos2,db,db2,depth+1,true);

        if(depth<=SAVE_DEPTH) db[pos.board]+=res;
        else if(depth==cnt+1) db2[pos.board]+=res;
        return res;
    }
        
    reversi pos2=pos;
    pos2.move(t[rng[depth].get()%j]);
    res=mcts(pos2,db,db2,depth+1);

    if(depth<=SAVE_DEPTH) db[pos.board]+=res;
    else if(depth==cnt+1) db2[pos.board]+=res;
    return res;
}

void *mcts_thread(void *arg){
    long long* N=(long long*)arg;
    int n=N[1];
    reversi **pos=(reversi**)(arg);
    for(int i=0;i<ACTUAL_COUNT;++i){
        reversi pos2=**pos;
        mcts(pos2,db[n],db2[n],cnt);
    }
    pthread_exit(nullptr);
}

void mcts_driver(reversi& pos){
    long long res=0;
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

int premcts(reversi &pos){
    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    return j;
}

void make_move(reversi& pos,int usecs,int cnt){
    int c=premcts(pos);
    if(c==0){
        cout << "IDO -1 -1" << endl;
        pos.move(-1);
        return;
    }
    
    ACTUAL_COUNT=1+COUNT*1000*(2+cnt*cnt/300)/(c*(80-cnt));
    itimerval T;
    memset(&T,0,sizeof(T));
    T.it_value.tv_usec=usecs%1000000;
    T.it_value.tv_sec=usecs/1000000;
    
    int x=-1;
    assert(setitimer(ITIMER_REAL,&T,nullptr)>=0);
    
    if(!sigsetjmp(env,1)) mcts_driver(pos);

    memset(&T,0,sizeof(T));
    setitimer(ITIMER_REAL,&T,nullptr);

    bool black=pos.board[64]==1;
    frac res=(black ? frac{0,1} : frac{1,1});
    for(int i=0;i<64;++i){
        if(!pos.check_move(i)) continue;
        reversi pos2=pos;
        pos2.move(i);
        frac d=database[pos2.board];
        if(black && d>res){
            res=d;
            x=i;
        }
        else if(!black && d<res){
            res=d;
            x=i;
        }
    }

    cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << endl;
    pos.move(x);
}

int main(){
    string s;
    reversi g;

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
            ++cnt;
            // cerr << g.board[64]-1 << " " << cnt << " " << (cnt&1) << endl;
            // assert(g.board[64]-1==(cnt&1));
            make_move(g,min(900000*a,500000*b),cnt);
            ++cnt;
            // g.print();
            // assert(g.board[64]-1==(cnt&1));
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
            make_move(g,4000000,cnt);
            ++cnt;
            // g.print();
            // assert(g.board[64]==2);
            // assert(cnt==1);
        }
    }

    // on_exit();
}