#ifndef TTT_H
#define TTT_H

#include "UTTTGame.h"

using Board = std::array<Player, 9>;

/// This class defines regular Tic-Tac-Toe game-flow and helper functions
class TTTGame {
public:
    static int wins [8][3];

    static std::vector<int> GetMoves(const Board & b);
    static Player GetWinner(const Board & b);
    static Player IsWinnableForPlayer(const Board &b);
    static Board DoMove(const Board &b, int &m, const Player &p);
    static std::vector<int> GetCellsOccupiedByPlayer(const Board & b, const Player & p);

    static int GetMinimumMovesToWin(const std::array<Player, 9> &board, const Player &player);
};


#endif
