// "ttt.h"
// Jeffrey Drost

#ifndef TTT_H
#define TTT_H

#include "uttt.h"

class ttt {
public:
    static std::vector<int> GetMoves(const MicroState & b);
    static Player GetWinner(const MicroState & b);
    static int CheckSetups(const MicroState &b, const Player &player);
    static Player IsWinnableBy(const MicroState & b);
};

#endif //TTT_H
