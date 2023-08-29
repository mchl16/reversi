#include <bits/stdc++.h>
#include "eval.h"

using namespace std;

double piece_diff(const reversi &pos){
    double res=0,res2=0;
    constexpr int T[]={0,1,-1,0},T2[]={0,1,1,0};
    for(int i=0;i<64;++i){
        assert(pos.board[i]<4 && pos.board[i]>=0);
        res+=T[pos.board[i]];
        res2+=T2[pos.board[i]];
    }
    //assert(res-res2<64 && res-res2>-64);
    return res;//res2;
}

double mobility_diff(reversi &pos){
    double res=0,res2=0;

    int b=pos.board[64];
    pos.board[64]=1;
    for(int i=0;i<64;++i) if(pos.check_move(i)) ++res;
    pos.board[64]=2;
    for(int i=0;i<64;++i) if(pos.check_move(i)) ++res2;
    pos.board[64]=b;

    return (res-res2)/(res+res2+1);
}

int table_heuristic(const reversi& pos){
    constexpr int T[]={100,-20,10,5,5,10,-20,100,\
                -20,-50,-2,-2,-2,-2,-50,-20,\
                10,-2,-1,-1,-1,-1,-2,10,\
                5,-2,-1,-1,-1,-1,-2,5,\
                5,-2,-1,-1,-1,-1,-2,5,\
                10,-2,-1,-1,-1,-1,-2,10,\
                -20,-50,-2,-2,-2,-2,-50,-20,\
                100,-20,10,5,5,10,-20,100};
    constexpr int T2[]={0,1,-1,0};

    long long res=0;
    for(int i=0;i<64;++i) res+=T2[pos.board[i]]*T[i];
    return res;
}

int end_of_group(const reversi& pos,int x,int y,const int *i){
    int g1=x+i[0],g2=y+i[1];
    for(;reversi::in_bounds(g1,g2) && (pos.board[8*g1+g2]==pos.board[8*x+y]);g1+=i[0],g2+=i[1]);
    if(!reversi::in_bounds(g1,g2)) return 3;
    return pos.board[8*g1+g2];
}

int get_stability(const reversi& pos,int x,int y){
    int t[8];
    for(int i=0;i<8;++i) t[i]=end_of_group(pos,x,y,reversi::gen[i]);
    constexpr int t2[4][2]={{0,2},{1,3},{4,6},{5,7}};
    for(const auto& j:t2){
        int a=t[j[0]],b=t[j[1]];
        if(a>b) swap(a,b);
        if(a==0 && b==3-pos.board[8*x+y]) return 0;
    }

    for(const auto& j:t2){
        int a=t[j[0]],b=t[j[1]];
        if(a==0 && b==0) return 1;
    }

    return 2;
}

double stability_diff(reversi& pos){
    constexpr int T[]={5,1,-3};
    double res=0,res2=0;

    int b=pos.board[64];
    pos.board[64]=1;
    for(int i=0;i<64;++i) if(pos.board[i]==1) res+=T[get_stability(pos,i>>3,i&7)];
    pos.board[64]=2;
    for(int i=0;i<64;++i) if(pos.board[i]==2) res2+=T[get_stability(pos,i>>3,i&7)];
    pos.board[64]=b;
    
    return (res-res2)/(res+res2+1);
}

double eval(reversi &pos){
    double t[4];
    t[0]=piece_diff(pos);
    t[1]=mobility_diff(pos);
    t[2]=table_heuristic(pos);
    t[3]=stability_diff(pos);

    double res=0;
    for(int i=0;i<4;++i) res+=default_weights[i]*t[i];
    return res;
}

double corner_diff(reversi& pos){
    constexpr int T[]={0,7,63,56};
    double res=0,res2=0;

    int b=pos.board[64];
    pos.board[64]=1;
    for(int i=0;i<4;++i) if(pos.check_move(T[i])) ++res;
    pos.board[64]=2;
    for(int i=0;i<4;++i) if(pos.check_move(T[i])) ++res2;
    pos.board[64]=b;
    
    return (res-res2)/(res+res2+1);
}

double eval2(reversi &pos){
    double t[5];
    t[0]=piece_diff(pos);
    t[1]=mobility_diff(pos);
    t[2]=table_heuristic(pos);
    t[3]=stability_diff(pos);
    t[4]=corner_diff(pos);

    int cnt=0;
    for(int i=0;i<64;++i) if(pos.board[i]!=3) ++cnt;

    double res=0;
    for(int i=0;i<5;++i) res+=((60-cnt)*default_weights[i]+cnt*default_weights2[i])*t[i];
    return res;
}

void eval3(reversi &pos){
    double t[5];
    t[0]=piece_diff(pos);
    t[1]=mobility_diff(pos);
    t[2]=table_heuristic(pos);
    t[3]=stability_diff(pos);
    t[4]=corner_diff(pos);

    int cnt=0;
    for(int i=0;i<64;++i) if(pos.board[i]!=3) ++cnt;

    cerr << "Eval:" << endl;
    for(int i=0;i<5;++i){
        cerr << ((60-cnt)*default_weights[i]+cnt*default_weights2[i]) << " " << t[i] << " ";
        cerr << ((60-cnt)*default_weights[i]+cnt*default_weights2[i])*t[i] << "!" << endl;
    }

    double res=0;
    for(int i=0;i<5;++i) res+=((60-cnt)*default_weights[i]+cnt*default_weights2[i])*t[i];
    cerr << "Total: " << res << endl;
}