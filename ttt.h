// "ttt.h"
// Jeffrey Drost

#ifndef TTT_H
#define TTT_H

#include "uttt.h"

class ttt {
public:
    static std::vector<int> GetMoves(const MicroState & b);
    static Player GetWinner(const MicroState & b);
    static bool CloseWin(const MicroState & b, const Player &me, const bool myWin);
    static Player IsWinnableBy(const MicroState & b);
};

#endif //TTT_H
