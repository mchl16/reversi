#include <bits/stdc++.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include "utility/mcts/mcts.h"
#include "utility/eval/eval.h"
#include "utility/basegame/reversi.h"
using namespace std;

bool passed=false;

pair<int,int> calc_moves(const reversi& pos){
    pair<int,int> res={0,-1};
    for(int i=0;i<64;++i) if(pos.check_move(i)){
        ++res.first;
        res.second=i;
    }
    return res;
}


//mcts
void make_move(reversi& pos,int usecs,int cnt){
    pair<int,int> c=calc_moves(pos);
    if(c.first==0){
        cout << "IDO -1 -1" << endl;
        pos.move(-1);
        return;
    }
    else if(c.first==1){
        cout << "IDO " << c.second%8 << " " << c.second/8 << endl;
        pos.move(c.second);
        return;
    }
    
    int x=mcts_driver(pos,cnt,usecs,passed);

    passed=(x<0);

    cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << endl;
    pos.move(x);
}

int main(){
    string s;
    reversi g;
    int cnt=0;
    passed=false;

    cout << "RDY" << endl;
    for(;;){
        cin >> s;
        if(s=="HEDID"){
            double a,b;
            int c,d;
            cin >> a >> b >> c >> d;
            g.move(8*d+c);
            ++cnt;
            make_move(g,min(900000*a,500000*b),cnt);
            ++cnt;
        }
        else if(s=="ONEMORE"){
            g=reversi();
            cnt=0;
            passed=false;
            cout << "RDY" << endl;
        }
        else if(s=="BYE"){
            exit(0);
        }
        else if(s=="UGO"){
            double a,b;
            cin >> a >> b;
            make_move(g,min(900000*a,500000*b),cnt);
            ++cnt;
        }
    }

    // on_exit();
}