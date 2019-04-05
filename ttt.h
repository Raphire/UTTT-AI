// "ttt.h"
// Jeffrey Drost

#ifndef UTTTPROBESTBOTEUW_TTT_H
#define UTTTPROBESTBOTEUW_TTT_H

#include "uttt.h"

class ttt {
public:
    static int EvaluateBoard(const MicroState & b, const Player & maximize);
    static std::vector<int> GetMoves(const MicroState & b);
    static std::vector<MicroState> GetChildren(const MicroState & b);
    static Player GetWinner(const MicroState & b);
    static bool CloseWin(const MicroState & b, const Player &me, const bool myWin);
    static Player GetPlayer(const MicroState & b);
    static int PlayerCount(const MicroState & b, const Player & p);
    static bool IsGameOver(const MicroState & b);
    static Player IsWinnableBy(const MicroState & b);
    static MicroState DoMove(const MicroState &b, int &m, const Player &p);
};


#endif //UTTTPROBESTBOTEUW_TTT_H
