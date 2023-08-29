#ifndef EVAL_H
#define EVAL_H

#include "../basegame/reversi.h"

// piece_diff
// mobility_diff
// table_heuristic
// stability_diff
// corner_diff
constexpr double default_weights[]={0.001,200,50,200,200};
constexpr double default_weights2[]={10,0,20,200,200};

double eval(reversi &pos);

double eval2(reversi &pos);

void eval3(reversi &pos);

#endif