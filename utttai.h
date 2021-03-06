// utttai.h
// Jeffrey Drost

#ifndef UTTTAI_H
#define UTTTAI_H

#include "utttbot.h"
#include "uttt.h"
#include "ttt.h"

#define INITIAL_SEARCH_DEPTH 1

struct MacroState {
    int x = -1;
    int y = -1;
};

class UTTTAI {
    static std::vector<Move> EvaluateBestMoves(const State &state, const std::vector<Move> &bestMoves, const Player &me);

    static int EvaluateState(const State &state, const Player &player);
    static int EvaluateMicroState(const MicroState &state, const Player &player);
    static int EvaluateNextPossibilities(const MicroState &state, const Player &me);

    static std::vector<State> GetChildStates(const State &state);
    static MicroState GetMicroState(const State &state, const Move &move, const bool getNext);
    static std::vector<MacroState> GetPreferredMacroBoards (const State &state, const Player &player, const int num);

public:
    static Move findBestMove(const State &state, const int &timeout, const int &timePerMove);
};

#endif //UTTTAI_H
