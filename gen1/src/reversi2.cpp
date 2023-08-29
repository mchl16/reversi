#include <bits/stdc++.h>
#include <sys/wait.h>
using namespace std;

constexpr int DEPTH=1;
constexpr int DIFF=DEPTH;
constexpr int COUNT=300;
constexpr int CPUCOUNT=4;

constexpr int gen[8][2]={{1,0},{0,1},{-1,0},{0,-1},{1,1},{-1,1},{-1,-1},{1,-1}};

// struct revtable{
//     bitset<128> data;
    
//     int& operator[]()
// }

struct reversi{
    char board[64];
    char color; 

    reversi() : color(1){
        memset(board,0,64);
        board[27]=2;
        board[28]=1;
        board[35]=1;
        board[36]=2;
    }

    static bool in_bounds(int x,int y){
        if(x<0) return false;
        if(x>=8) return false;
        if(y<0) return false;
        return y<8;
    }

    void place_piece(int field){
        board[field]=color;
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-color);g1+=i[0],g2+=i[1]);
            if(!in_bounds(g1,g2) || board[8*g1+g2]!=color) continue;
            for(g1=f1+i[0],g2=f2+i[1];in_bounds(g1,g2) && board[8*g1+g2]==3-color;g1+=i[0],g2+=i[1]){
                board[8*g1+g2]=color;
            }
        }
    }

    bool check_move(int field) const{
        if(field<0) return true;
        if(board[field]!=0) return false;
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-color);g1+=i[0],g2+=i[1]);
            if(in_bounds(g1,g2) && board[8*g1+g2]==color){
                if(g1!=f1+i[0] || g2!=f2+i[1]) return true;
            }
        }
        return false;
    }

    void move(int field){
        if(field>=0) place_piece(field);
        color=3-color;
    }
};

long long rankpos(const reversi& pos){
    constexpr int T[]={100,-20,10,5,5,10,-20,100,\
                -20,-50,-2,-2,-2,-2,-50,-20,\
                10,-2,-1,-1,-1,-1,-2,10,\
                5,-2,-1,-1,-1,-1,-2,5,\
                5,-2,-1,-1,-1,-1,-2,5,\
                10,-2,-1,-1,-1,-1,-2,10,\
                -20,-50,-2,-2,-2,-2,-50,-20,\
                100,-20,10,5,5,10,-20,100};
    constexpr int T2[]={0,1,-1};
    long long res=0;
    for(int i=0;i<64;++i) res+=T2[pos.board[i]]*T[i];
    return res;
}



random_device rd;
mt19937 generator(rd());
int forkcount=0;

int mcts(reversi& pos){
    bool passed=false;
    for(;;){
        int t[64];
        int j=0;
        for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

        if(j==0){
            if(passed){
                constexpr int T2[]={0,1,-1};
                long long res2=0;
                for(int i=0;i<64;++i) res2+=T2[pos.board[i]];
                if(res2>0) res2=1;
                else if(res2<0) res2=-1;
                return res2;
            }
            passed=true;
            continue;
        }
        
        uniform_int_distribution<int> distribution(0,j-1);
        pos.move(t[distribution(generator)]);
    }
}

long long mcts_driver(reversi& pos){
    int pipes[CPUCOUNT][2];
    int pids[CPUCOUNT];
    char buffer[8];
    long long res=0;

    for(int i=0;i<CPUCOUNT;++i) pipe(pipes[i]);
    for(int i=0;i<CPUCOUNT;++i){
        pids[i]=fork();
        if(pids[i]==0){
            close(pipes[i][0]);
            long long res2=0;
            for(int j=0;j<COUNT;++j){
                reversi pos2=pos;
                res2+=mcts(pos2);
            }
            sprintf(buffer,"%lld",res2);
            write(pipes[i][1],buffer,strlen(buffer));
            exit(0);
        }
        else ++forkcount;
        close(pipes[i][1]);
    }

    for(int i=0;i<CPUCOUNT;++i) waitpid(pids[i],nullptr,0);
    for(int i=0;i<CPUCOUNT;++i){
        long long res2;
        read(pipes[i][0],buffer,8);
        sscanf(buffer,"%lld",&res2);
        res+=res2;
        close(pipes[i][0]);
    }
    return res;
}

pair<long long,int> alphabeta(reversi& pos,long long a=-1e18,long long b=1e18,int depth=DEPTH,int last=-1,bool passed=false){
    bool black=(pos.color==1);
    long long val=rankpos(pos);
    int res=last;
    if(depth==0) return {mcts_driver(pos),res};

    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(passed){
            constexpr int T2[]={0,1,-1};
            long long res2=0;
            for(int i=0;i<64;++i) res2+=T2[pos.board[i]];
            if(res2>0) res2=1;
            else if(res2<0) res2=-1;
            return {1e18*res2,-1};
        }
        pos.move(-1);
        return alphabeta(pos,a,b,depth-1,-1,true);
    }

    reversi T[64];
    int ranks[64];
    for(int i=0;i<j;++i){
        T[t[i]]=pos;
        T[t[i]].move(t[i]);
        ranks[t[i]]=rankpos(T[t[i]]);
    }
    sort(t,t+j,[&ranks](int a,int b){return ranks[a]>ranks[b];});

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

int main(){
    string s;
    reversi g;
    cout << "RDY\n";
    for(;;){
        cin >> s;
        if(s=="HEDID"){
            float a,b;
            int c,d;
            cin >> a >> b >> c >> d;
            g.move(8*d+c);
            auto x=alphabeta(g).second;
            cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << "\n";
            g.move(x);
        }
        else if(s=="ONEMORE"){
            g=reversi();
            cerr << forkcount << "\n";
            forkcount=0;
            cout << "RDY\n";
        }
        else if(s=="BYE"){
            cerr << forkcount << "\n";
            forkcount=0;
            exit(0);
        }
        else if(s=="UGO"){
            auto x=alphabeta(g).second;
            cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << "\n";
            g.move(x);
        }
    }
}