#ifndef MCTS_H
#define MCTS_H

#include <time.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>
#include <setjmp.h>
#include <boost/unordered_map.hpp>

#include "../basegame/reversi.h"
#include "../threads/threads.h"
#include "database.h"

namespace mcts{
    extern int actual_cnt;
    extern sigjmp_buf env;

    constexpr long long COUNT=500000;
    constexpr int THRESHOLD=40;
}

int mcts_driver(reversi& pos,int depth,int usecs,bool passed=false);

frac mcts_playout(reversi& pos,int n,int depth,bool passed=false);

frac mcts_playout_pass(reversi& pos,int n,int depth,tree_database::data_t& record);

#endif