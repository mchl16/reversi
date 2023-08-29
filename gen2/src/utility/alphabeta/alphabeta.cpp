#include "alphabeta.h"
#include "../eval/eval.h"

#include <time.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>
#include <setjmp.h>

using namespace std;

namespace ab{
    int abdepth=1;
    int cnt=0;
    int last_cnt=0;
    sigjmp_buf env;

    void sigalrm_handler(int s){
        cerr << "Nie dali mi :C" << endl;
        // for(int i=0;i<4;++i){
        //     pthread_cancel(tids[i]);
        //     pthread_join(tids[i],nullptr);
        // }
        siglongjmp(env,1);
    }
}

double alphabeta(reversi& pos,int depth,double a,double b,bool passed){
    ++ab::cnt;
    bool black=(pos.board[64]==1);
    if(depth==0) return eval2(pos);

    unsigned int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(depth>=ab::abdepth) return 0;
        if(passed){
            constexpr int T2[]={0,1,-1,0};
            long long res2=0;
            for(int i=0;i<64;++i) res2+=T2[pos.board[i]];
            if(res2>0) return 1e18;
            else if(res2<0) return -1e18;
            return 0;
        }
        else{
            pos.move(-1);
            return alphabeta(pos,depth-1,a,b,true);
        }
    }
    else if(j==1){
        if(depth>=ab::abdepth) return t[0];
        return eval2(pos);
    }

    reversi T[64];
    double ranks[64];
    for(int i=0;i<j;++i){
        T[t[i]]=pos;
        T[t[i]].move(t[i]);
        ranks[t[i]]=eval2(T[t[i]]);
    }
    
    if(black) sort(t,t+j,[&ranks](int a,int b){return ranks[a]>ranks[b];});
    else sort(t,t+j,[&ranks](int a,int b){return ranks[a]<ranks[b];});

    double val=0;
    int res=t[0];
    for(int i=0;i<j;++i){
        auto ab=alphabeta(T[t[i]],depth-1,a,b,false);

        if(black){
            if(ab>a){
                a=ab;
                val=a;
                res=t[i];
            }
        }
        else{
            if(ab<b){
                b=ab;
                val=b;
                res=t[i];
            }
        }
        if(a>=b) break;
    }
    return depth==ab::abdepth ? res : val;
}

void iterative_alphabeta(reversi& pos,int usecs,int& res){
    timespec t0;
    clock_gettime(CLOCK_MONOTONIC,&t0);
    ab::cnt=1;

    struct sigaction sa2;
    sa2.sa_handler=ab::sigalrm_handler;
    sigaction(SIGALRM,&sa2,NULL);

    itimerval T;
    memset(&T,0,sizeof(T));
    T.it_value.tv_usec=usecs%1000000;
    T.it_value.tv_sec=usecs/1000000;
    assert(setitimer(ITIMER_REAL,&T,nullptr)>=0);

    if(sigsetjmp(ab::env,1));
    else for(int i=1;;++i){
        //cerr << i << "!" << endl;
        ab::last_cnt=ab::cnt;
        ab::cnt=0;
        ab::abdepth=i;
        res=alphabeta(pos,i);
        if(ab::cnt==ab::last_cnt) break; //endgame, we actually managed to explore all possibilities

        timespec t;
        clock_gettime(CLOCK_MONOTONIC,&t);
        long long duration=1000000LL*(t.tv_sec-t0.tv_sec)+(t.tv_nsec-t0.tv_nsec)/1000;
        //cerr << duration << "?!" << endl;
        if(duration*ab::cnt/ab::last_cnt>usecs) break;
    }

    memset(&T,0,sizeof(T)); //cleaning up
    setitimer(ITIMER_REAL,&T,nullptr);
}