//
// Created by Jorn on 04/04/2019.
//

#ifndef UTTTPROBESTBOTEUW_TTTAI_H
#define UTTTPROBESTBOTEUW_TTTAI_H


#include "ttt.h"

class TTTAI {
public:
    /// Rates effectiveness of given move in a regular game of tic-tac-toe
    static std::array<int, 9> RateMoves(const Board &board, Player turn);

    /// Function should exclusively be used for Player::X or Player::O on unfinished games
    static std::vector<int> GetWinningMoves(const Board & board, Player p);

    /// Finds moves that could set-up a potential win (moves that connect with other cells of Player p)
    static std::vector<int> GetSetupMoves(const Board &board, Player p);
};


#endif //UTTTPROBESTBOTEUW_TTTAI_H
