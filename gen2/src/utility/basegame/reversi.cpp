#include "reversi.h"

bool reversi::in_bounds(int x,int y){
    if(x<0) return false;
    if(x>=8) return false;
    if(y<0) return false;
    return y<8;
}

void reversi::place_piece(int field){
    board[field]=turn;
    int f1=field>>3,f2=field&7;
    for(const auto& i:gen){
        int g1=f1+i[0],g2=f2+i[1];
        for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-turn);g1+=i[0],g2+=i[1]);
        if(!in_bounds(g1,g2) || board[8*g1+g2]!=turn) continue;
        for(g1=f1+i[0],g2=f2+i[1];in_bounds(g1,g2) && board[8*g1+g2]==3-turn;g1+=i[0],g2+=i[1]){
            board[8*g1+g2]=turn;
        }
    }
}

bool reversi::check_move(int field) const{
    if(board[field]!=3) return false;
    int f1=field>>3,f2=field&7;
    for(const auto& i:gen){
        int g1=f1+i[0],g2=f2+i[1];
        for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-turn);g1+=i[0],g2+=i[1]);
        if(in_bounds(g1,g2) && board[8*g1+g2]==turn){
            if(g1!=f1+i[0] || g2!=f2+i[1]) return true;
        }
    }
    return false;
}

#include <assert.h>
using namespace std;

void reversi::move(int field){
    assert(field>=0 || !passed);
    if(field>=0){
        if(!check_move(field)){
            print();
            cerr << field << " " << (".o#."[turn]) <<  "!!!" << endl;
            assert(check_move(field));
        }
        place_piece(field);
        passed=false;
    }
    else passed=true;

    assert(passed==(field<0));
    turn=3-turn;
}



void reversi::print(){
    for(int i=0;i<8;++i){
        for(int j=0;j<8;++j){
            const char *c=".o#.";
            cerr << c[board[8*i+j]];
        }
        cerr << "\n";
    }
    cerr << endl;
}