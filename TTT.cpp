#include "TTT.h"

int TTT::wins [8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6}
};

Player TTT::GetWinner(const Board &b)
{
    // Horizontal
    if(b[0] == b[1] && b[1] == b[2] && b[2] != Player::None) return b[0];
    if(b[3] == b[4] && b[4] == b[5] && b[5] != Player::None) return b[3];
    if(b[6] == b[7] && b[7] == b[8] && b[8] != Player::None) return b[6];

    // Vertical
    if(b[0] == b[3] && b[3] == b[6] && b[6] != Player::None) return b[0];
    if(b[1] == b[4] && b[4] == b[7] && b[7] != Player::None) return b[1];
    if(b[2] == b[5] && b[5] == b[8] && b[8] != Player::None) return b[2];

    // Diagonal
    if(b[0] == b[4] && b[4] == b[8] && b[8] != Player::None) return b[0];
    if(b[2] == b[4] && b[4] == b[6] && b[6] != Player::None) return b[2];

    return  Player::None;
}

std::vector<int> TTT::GetMoves(const Board &b)
{
    std::vector<int> moves = {};
    if(GetWinner(b) != Player::None) return moves;
    else for(int m = 0; m < 9; m++)
        if(b[m] == Player::None) moves.push_back(m);
    return moves;
}

Board TTT::DoMove(const Board &b, int &m, const Player &p)
{
    Board newBoard = b;
    newBoard[m] = p;
    return newBoard;
}

Player TTT::IsWinnableForPlayer(const Board &b)
{
    Player winner = GetWinner(b);
    if(winner != Player::None) return winner;

    Player winnableBy = Player::None;

    // Check if each win pos can still be occupied by each player
    for (auto & win : wins)
    {
        if(b[win[0]] != Player::X && b[win[1]] != Player::X && b[win[2]] != Player::X) // If player O can win this win
            if(winnableBy == Player::X) return Player::Both;
            else winnableBy = Player::O;

        if(b[win[0]] != Player::O && b[win[1]] != Player::O && b[win[2]] != Player::O) // If player O can win this win
            if(winnableBy == Player::O) return Player::Both;
            else winnableBy = Player::X;
    }
    return winnableBy;
}


std::vector<int> TTT::GetCellsOccupiedByPlayer(const Board & b, const Player & p)
{
    std::vector<int> cells;
    for(int c = 0; c < 9; c++)
        if(b[c] == p) cells.push_back(c);

    return cells;
}

/// Returns 0 if board has been decided or no winning moves exist for given player
int TTT::GetMinimumMovesToWin(const std::array<Player, 9> &board, const Player &player)
{
    if(GetWinner(board) != Player::None) return 0;

    int result = 0;
    Player opponent = player == Player::X ? Player::O : Player::X;

    for (auto & win : wins)
    {
        int pCount = 0;
        if (board[win[0]] != opponent && board[win[1]] != opponent && board[win[2]] != opponent)
        {
            if(board[win[0]] == player) pCount++;
            if(board[win[1]] == player) pCount++;
            if(board[win[2]] == player) pCount++;

            int num = 3 - pCount;
            if(num < result || result == 0) result = num;
        } else continue;
    }

    return result;
}
