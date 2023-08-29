# reversi

This repo contains several AI agents capable of playing Reversi. They are structured as follows:

-`bots` - a few bots of varying strength for testing purposes
-`gen1` - uses shallow alpha-beta pruning combined with parallel primitive Monte Carlo Tree Search
-`gen2` (in development) - uses Monte Carlo Tree Search, the long-term goal is to parallelize it and add neural network evaluation

Bots can be tested using a provided Python program (60s per player, 4s per move).
