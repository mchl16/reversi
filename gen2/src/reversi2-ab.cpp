#include <bits/stdc++.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include "utility/alphabeta/alphabeta.h"
#include "utility/eval/eval.h"
#include "utility/basegame/reversi.h"
using namespace std;

int calc_moves(const reversi& pos){
    int res=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) ++res;
    return res;
}


//alpha-beta with iterative deepening
void make_move(reversi& pos,int usecs,int cnt){
    int c=calc_moves(pos);
    if(c==0){
        cout << "IDO -1 -1" << endl;
        pos.move(-1);
        return;
    }
    
    int x=-1;
    iterative_alphabeta(pos,usecs,x);

    cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << endl;
    pos.move(x);
}

int main(){
    string s;
    reversi g;
    int cnt=0;

    cout << "RDY" << endl;
    for(;;){
        cin >> s;
        if(s=="HEDID"){
            double a,b;
            int c,d;
            cin >> a >> b >> c >> d;
            // cerr << b << "!" << endl;
            g.move(8*d+c);
            eval3(g);
            // g.print();
            ++cnt;
            // cerr << g.board[64]-1 << " " << cnt << " " << (cnt&1) << endl;
            // assert(g.board[64]-1==(cnt&1));
            make_move(g,min(900000*a,500000*b),cnt);
            eval3(g);
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
            double a,b;
            cin >> a >> b;
            make_move(g,min(900000*a,500000*b),cnt);
            eval3(g);
            ++cnt;
            // g.print();
            // assert(g.board[64]==2);
            // assert(cnt==1);
        }
    }

    // on_exit();
}