//
// Created by Jorn on 03/04/2019.
//

#include "ttt.h"

int ttt::wins [8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6}
};

Player ttt::GetWinner(const Board &b)
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

std::vector<int> ttt::GetMoves(const Board &b)
{
    std::vector<int> moves = {};
    if(GetWinner(b) != Player::None) return moves;
    else for(int m = 0; m < 9; m++)
        if(b[m] == Player::None) moves.push_back(m);
    return moves;
}

Player ttt::GetPlayer(const Board & b)
{
    int x = PlayerCount(b, Player::X);
    int o = PlayerCount(b, Player::O);

    if(x + o == 9) return Player::None;

    int d = x - o;
    if(d > 0) return Player::O;

    return Player::X;
}

std::vector<Board> ttt::GetChildren(const Board &b)
{
    std::vector<Board> children = {};

    std::vector<int> moves = GetMoves(b);
    Player turn = GetPlayer(b);

    for(int m = 0; m < moves.size(); m++)
        children.push_back(DoMove(b, moves[m], turn));

    return children;
}

Board ttt::DoMove(const Board &b, int &m, const Player &p)
{
    Board newBoard = b;
    newBoard[m] = p;
    return newBoard;
}

int ttt::PlayerCount(const Board &b, const Player &p)
{
    int c = 0;
    for(int i = 0; i < 9; i++)
        if(b[i] == p) c++;
    return c;
}

Player ttt::IsWinnableForPlayer(const Board &b)
{
    if(GetWinner(b) != Player::None) return Player::None;

    Player winnableBy = Player::None;
    int wins[8][3] = {
            {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
            {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
            {0, 4, 8}, {2, 4, 6}
    };

    // Check if each win pos can still be occupied by each player
    for(int w = 0; w < 8; w++)
    {
        if(b[wins[w][0]] != Player::X && b[wins[w][1]] != Player::X && b[wins[w][2]] != Player::X) // If player O can win this win
            if(winnableBy == Player::X) return Player::Both;
            else winnableBy = Player::O;

        if(b[wins[w][0]] != Player::O && b[wins[w][1]] != Player::O && b[wins[w][2]] != Player::O) // If player O can win this win
            if(winnableBy == Player::O) return Player::Both;
            else winnableBy = Player::X;
    }
    return winnableBy;
}

bool ttt::IsGameOver(const Board &b)
{
    if(GetMoves(b).size() == 0)
        return true;
    return false;
}

std::vector<int> ttt::GetCellsOccupiedByPlayer(const Board & b, const Player & p)
{
    std::vector<int> cells;
    for(int c = 0; c < 9; c++)
        if(b[c] == p) cells.push_back(c);

    return cells;
}
