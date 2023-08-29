#include "mcts.h"

using namespace std;

namespace mcts{
    int actual_cnt=0;
    sigjmp_buf env;
    
    void sigalrm_handler(int s){
        cerr << "Timeout :C" << endl;
        // for(int i=0;i<4;++i){
        //     pthread_cancel(tids[i]);
        //     pthread_join(tids[i],nullptr);
        // }
        // actual_cnt=0;
        siglongjmp(env,1);
    }
}

int pick_move(const reversi& pos,bool white_move){
    frac r=frac(-1,1);
    int m=-1;

    int s=1;
    if(white_move) r.num=1,s=-1;

    for(int i=0;i<64;++i){
        if(pos.check_move(i)){
            reversi pos2=pos;
            pos2.move(i);
            auto& record=game_db[pos2];
            if (record.val.den==0){
                if(record.val.num==s) return i;
                
                if(white_move){
                    if(frac(record.val.num,1)<=r){
                        r=frac(record.val.num,1);
                        m=i;
                    } 
                }
                else if(frac(record.val.num,1)>=r){
                    r=frac(record.val.num,1);
                    m=i;
                } 
            }
            else{
                if(white_move){
                    if(record.val<=r){
                        r=record.val;
                        m=i;
                    } 
                }
                else if(record.val>=r){
                    r=record.val;
                    m=i;
                } 
            }
        }
    }
    return m;
}

int get_result(const reversi& pos,bool white_move){
    int m=white_move ? 1 : -1;

    for(int i=0;i<64;++i){
        if(pos.check_move(i)){
            reversi pos2=pos;
            pos2.move(i);
            int record=game_db[pos2].val.num;
            if(white_move) m=max(m,record);
            else m=min(m,record);
        }
    }
    return m;
}

frac mcts_playout_pass(reversi& pos,int n,int depth,bool passed,tree_database::data_t& record){
    frac res=frac(0,0);

    if(passed){ //game over
        constexpr int T2[]={0,1,-1,0};
        int x=0;
        for(int i=0;i<64;++i) x+=T2[pos.board[i]];
        
        if(x<0) res.num=-1;
        else if(x>0) res.num=1;

        record.val=res;
        res.den=1;
        return res;
    }

    reversi pos2=pos; //pass
    pos2.move(-1);

    auto& record2=game_db[pos2];

    if(record2.explored){ //tree fully explored after passing 
        record.val=record2.val;
        return frac(record.val.num,1);
    }

    res=mcts_playout(pos2,n,depth+1,true);
    
    if(record2.val.den==0){ //propagate guaranteed result
        record.val=record2.val;
        return res;
    }

    record.val+=res;
    return res;
}

frac mcts_playout(reversi& pos,int n,int depth,bool passed){
    // cerr << depth << " " << passed << endl;
    // pos.print();
    
    // cerr << passed << " " << (int)pos.board[65] << endl;
    assert(passed==pos.passed);
    assert(depth<130);

    frac res=frac(0,0);
    auto& record=game_db[pos];
    
    if(!record.visited){ //generate all legal and unexplored moves from a given position
        record.visited=true;

        for(int i=0;i<64;++i){
            if(!pos.check_move(i)) continue;

            reversi pos2=pos;
            pos2.move(i);

            auto& record2=game_db[pos2];

            if (record2.explored){ //explored move
                int s=(depth&1 ? -1 : 1);
                if(record2.val.num==s){  //guaranteed win, no need to init a structure
                    //record.prob.data.clear();
                    record.explored=true;
                    record.val=frac(s,0);
                    return frac(s,1);
                }
                else{ //add an explored move
                    record.add_move(i,&record2);
                    record.set_threads(i,-1);
                }
            }
            else record.add_move(i,&record2); //add an unexplored move
        }
    } 
    
    if(record.pick(depth&1)==-1){ //no unexplored move left - unless it's game over, we can only pass
        return mcts_playout_pass(pos,n,depth,passed,record);
    }

    if(record.all_explored()){ //other playouts checked all moves
        record.data.clear();
        record.explored=true;
        int res2=get_result(pos,depth&1);
        record.val=frac(res2,0);
        return frac(res2,1);
    }

    int x=record.pick(depth&1); //random playout

    assert(x>=0);
    if(depth>=100) cerr << x << "!!!" << endl;
    reversi pos2=pos; 
    pos2.move(x);
    res=mcts_playout(pos2,n,depth+1); 
    
    auto& record2=game_db[pos2];
    if(record2.explored){ //subtree fully explored
        record.set_threads(x,-1);
        int s=(depth&1 ? -1 : 1);
        if(record2.val.num==s){ //guaranteed win
            record.data.clear();
            record.explored=true;
            record.val=frac(s,0);
            return frac(s,1);
        }
        else if(record.all_explored()){ //whole subtree explored, we pick the best move
            record.data.clear();
            record.explored=true;
            record.val=frac(get_result(pos,depth&1),0);
            return res;
        }
    }

    record.val+=res;
    return res;
}

void *mcts_thread(void *arg){
    long long* N=(long long*)arg;
    int n=N[1];
    int depth=N[2];
    bool passed=N[3];
    reversi **pos=(reversi**)(arg);
    for(int i=0;i<mcts::actual_cnt;++i){
        reversi pos2=**pos;
        if(game_db[pos2].explored) break;
        //else if((i&65535)==0) cerr << flush << i << endl;
        mcts_playout(pos2,n,depth,passed);
    }
    pthread_exit(nullptr);
}

int mcts_driver(reversi& pos,int depth,int usecs,bool passed){
    
    mcts::actual_cnt=2137+mcts::COUNT*(2.0+(double)depth*depth/120)/(120-depth);

    cerr << mcts::actual_cnt << endl;

    pthread_t tids[threads::CPU_COUNT];

    constexpr int ARG_SIZE=4;
    void *args[ARG_SIZE*threads::CPU_COUNT];

    itimerval T;
    memset(&T,0,sizeof(T));
    T.it_value.tv_usec=usecs%1000000;
    T.it_value.tv_sec=usecs/1000000;

    memset(args,0,sizeof(args));
    for(int i=0;i<threads::CPU_COUNT;++i){
        args[ARG_SIZE*i]=&pos;
        memset(&args[ARG_SIZE*i+1],i,1);
        memset(&args[ARG_SIZE*i+2],depth,1);
        memset(&args[ARG_SIZE*i+3],passed,1);
    }

    struct sigaction sa2;
    sa2.sa_handler=mcts::sigalrm_handler;
    
    if(sigsetjmp(mcts::env,1)==0){
        sigaction(SIGALRM,&sa2,NULL);
        assert(setitimer(ITIMER_REAL,&T,nullptr)>=0);

        for(int i=0;i<threads::CPU_COUNT;++i){
            assert(pthread_create(&tids[i],nullptr,mcts_thread,&args[ARG_SIZE*i])==0);
        }

        for(int i=0;i<threads::CPU_COUNT;++i) pthread_join(tids[i],nullptr);
    }
    else for(int i=0;i<threads::CPU_COUNT;++i) pthread_cancel(tids[i]);
    
    memset(&T,0,sizeof(T)); //cleaning up
    setitimer(ITIMER_REAL,&T,nullptr);
    sa2.sa_handler=SIG_DFL;
    sigaction(SIGALRM,&sa2,NULL);
    
    return pick_move(pos,depth&1);
} 
