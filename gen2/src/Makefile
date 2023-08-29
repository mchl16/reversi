CC=g++
CFLAGS=-std=c++20 -O3 -march=native
OUT=reversi
SRC_PATH=./
OUT_PATH=../bin/

REVISION=4

DEPS=utility/basegame/reversi.cpp
DEPS_AB=utility/basegame/reversi.cpp utility/eval/eval.cpp

SRC=$(shell find . -name "*.cpp")
OBJ=$(patsubst %.cpp, %.o, $(SRC))

TARGET:=$(word 2,$(MAKECMDGOALS))

# .PHONY: clean distclean

mcts:
	$(CC) $(CFLAGS) $(DEPS) utility/mcts/mcts.cpp utility/mcts/database.cpp reversi2-mcts.cpp -o $(OUT_PATH)reversi2-mk$(REVISION)

alphabeta:
	$(CC) $(CFLAGS) $(DEPS_AB) utility/alphabeta/alphabeta.cpp reversi2-alphabeta.cpp -o $(OUT_PATH)reversi2-mk$(REVISION)

clean:
	$(RM) *.o
	$(RM) *.d

distclean:
	$(RM) *.o
	$(RM) *.d
	$(RM) $(TARGET)

# $(CC) $(CFLAGS) -c $< -o $@
# mv reversi2-$(TARGET).o reversi.o




