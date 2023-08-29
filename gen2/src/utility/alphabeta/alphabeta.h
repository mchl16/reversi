#ifndef ALPHABETA_H
#define ALPHABETA_H

#include "../basegame/reversi.h"
#include <algorithm>
#include <setjmp.h>

constexpr int DEPTH=3;

namespace ab{
    extern int cnt;
    extern int last_cnt;
    extern int abdepth;
    extern sigjmp_buf env;

    extern void sigalrm_handler(int s);
}

double alphabeta(reversi& pos,int depth=ab::abdepth,double a=-1,double b=1,bool passed=false);

void iterative_alphabeta(reversi& pos,int usecs,int& res);

#endif