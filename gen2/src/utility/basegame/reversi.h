#ifndef REVERSI_H
#define REVERSI_H

#include <cstring>
#include <iostream>

struct reversi{
    static constexpr int gen[8][2]={{1,0},{0,1},{-1,0},{0,-1},{1,1},{-1,1},{-1,-1},{1,-1}};
    static constexpr int black=1,white=2;
    
    char board[64];
    char turn;
    bool passed;

    reversi(){
        memset(board,3,64);
        board[27]=white;
        board[28]=black;
        board[35]=black;
        board[36]=white;
        turn=black;
        passed=false;
    }

    static bool in_bounds(int x,int y);

    void place_piece(int field);

    bool check_move(int field) const;

    void move(int field);

    void print();
};

#endif