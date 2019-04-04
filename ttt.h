//
// Created by Jorn on 03/04/2019.
//

#ifndef UTTTPROBESTBOTEUW_TTT_H
#define UTTTPROBESTBOTEUW_TTT_H

#include "uttt.h"

using Board = std::array<Player, 9>;

class ttt {
public:
    static int EvaluateBoard(const Board & b, const Player & maximize);
    static std::vector<int> GetMoves(const Board & b);
    static std::vector<Board> GetChildren(const Board & b);
    static Player GetWinner(const Board & b);
    static Player GetPlayer(const Board & b);
    static int PlayerCount(const Board & b, const Player & p);
    static bool IsGameOver(const Board & b);
    static Player IsWinnableBy(const Board & b);
    static Board DoMove(const Board &b, int &m, const Player &p);
};


#endif //UTTTPROBESTBOTEUW_TTT_H
