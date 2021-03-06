// "ttt.cpp"
// Jeffrey Drost

#include "ttt.h"

// Returns the winner (if any) of a given microboard
Player ttt::GetWinner(const MicroState &b)
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

// Checks if the given players has at least 1 occurrence of two in a row in the given microboard
int ttt::CheckSetups(const MicroState &b, const Player &player)
{
    int count = 0;

    // Horizontal
    if(b[0] == b[1] && b[2] == Player::None && b[0] == player) count++;
    if(b[3] == b[4] && b[5] == Player::None && b[3] == player) count++;
    if(b[6] == b[7] && b[7] == Player::None && b[6] == player) count++;
    if(b[0] == b[2] && b[1] == Player::None && b[0] == player) count++;
    if(b[3] == b[5] && b[4] == Player::None && b[3] == player) count++;
    if(b[6] == b[8] && b[7] == Player::None && b[6] == player) count++;
    if(b[1] == b[2] && b[0] == Player::None && b[1] == player) count++;
    if(b[4] == b[5] && b[3] == Player::None && b[4] == player) count++;
    if(b[7] == b[8] && b[6] == Player::None && b[7] == player) count++;

    // Vertical
    if(b[0] == b[3] && b[6] == Player::None && b[0] == player) count++;
    if(b[1] == b[4] && b[7] == Player::None && b[1] == player) count++;
    if(b[2] == b[5] && b[8] == Player::None && b[2] == player) count++;
    if(b[0] == b[6] && b[3] == Player::None && b[0] == player) count++;
    if(b[1] == b[7] && b[4] == Player::None && b[1] == player) count++;
    if(b[2] == b[8] && b[5] == Player::None && b[2] == player) count++;
    if(b[3] == b[6] && b[0] == Player::None && b[3] == player) count++;
    if(b[4] == b[7] && b[1] == Player::None && b[4] == player) count++;
    if(b[5] == b[8] && b[2] == Player::None && b[5] == player) count++;

    // Diagonal
    if(b[0] == b[4] && b[8] == Player::None && b[0] == player) count++;
    if(b[2] == b[4] && b[6] == Player::None && b[2] == player) count++;
    if(b[4] == b[8] && b[0] == Player::None && b[4] == player) count++;
    if(b[4] == b[6] && b[2] == Player::None && b[4] == player) count++;
    if(b[8] == b[0] && b[4] == Player::None && b[8] == player) count++;
    if(b[6] == b[2] && b[4] == Player::None && b[6] == player) count++;

    return count;
}

// Returns possible moves for a given microboard
std::vector<int> ttt::GetMoves(const MicroState &b)
{
    std::vector<int> moves = {};
    if(GetWinner(b) != Player::None) return moves;
    else for(int m = 0; m < 9; m++)
        if(b[m] == Player::None) moves.push_back(m);
    return moves;
}

// Returns what players would be able to still win the given microboard
Player ttt::IsWinnableBy(const MicroState &b)
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
