//
// Created by Jorn on 04/04/2019.
//

#include <algorithm>

#include "TTTAI.h"
#include "ttt.h"

std::array<int, 9> TTTAI::RateMoves(const Board &board, Player turn)
{
    std::array<int, 9> ratings = {-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000};
    std::vector<int> moves = ttt::GetMoves(board);

    Player other = turn == Player::X ? Player::O : Player::X;

    std::vector<int> winningMoves = GetWinningMoves(board, turn);
    std::vector<int> defendingMoves = GetWinningMoves(board, other);

    std::vector<int> playerSetupMoves = GetSetupMoves(board, turn);
    std::vector<int> otherSetupMoves = GetSetupMoves(board, other);

    for(int i = 0; i < moves.size(); i++)
    {
        int score = 0;

        // Attacking moves are rated higher than defending moves as you already implicitly defend by winning a board.
        if(std::find(winningMoves.begin(), winningMoves.end(), moves[i]) != winningMoves.end()) score += 2;
        // Attacking and defending simultaneously should however be rated even higher as you might not get
        // another chance to make a move on this board when an opposing win is imminent as well...
        if(std::find(defendingMoves.begin(), defendingMoves.end(), moves[i]) != defendingMoves.end()) score += 1;

        // Setting up wins should not be rewarded when winning moves exist due
        // to the eliminating nature of UTTTAI's selection algorithm as this
        // might lead to the unnecessary eliminations of otherwise viable moves.
        if(winningMoves.empty())
        {
            score += std::count(playerSetupMoves.begin(), playerSetupMoves.end(), moves[i]);
            score += std::count(otherSetupMoves.begin(), otherSetupMoves.end(), moves[i]);
        }
        ratings[moves[i]] = score;
    }

    return ratings;
}

std::vector<int> TTTAI::GetWinningMoves(const Board &board, Player p)
{
    std::vector<int> moves;

    for(int w = 0; w < 8; w++)
        if(board[ttt::wins[w][0]] == p && board[ttt::wins[w][1]] == p) moves.push_back(ttt::wins[w][2]);
        else if(board[ttt::wins[w][1]] == p && board[ttt::wins[w][2]] == p) moves.push_back(ttt::wins[w][0]);
        else if(board[ttt::wins[w][0]] == p && board[ttt::wins[w][2]] == p) moves.push_back(ttt::wins[w][1]);

    return moves;
}

std::vector<int> TTTAI::GetSetupMoves(const Board &board, Player p)
{
    std::vector<int> moves;
    std::vector<int> cells = ttt::GetCellsOccupiedByPlayer(board, p);

    for(int w = 0; w < 8; w++)
        if(board[ttt::wins[w][0]] == p && board[ttt::wins[w][1]] == Player::None && board[ttt::wins[w][2]] == Player::None) moves.push_back(ttt::wins[w][0]);
        else if(board[ttt::wins[w][0]] == Player::None && board[ttt::wins[w][1]] == p && board[ttt::wins[w][2]] == Player::None) moves.push_back(ttt::wins[w][1]);
        else if(board[ttt::wins[w][0]] == Player::None && board[ttt::wins[w][1]] == Player::None && board[ttt::wins[w][2]] == p) moves.push_back(ttt::wins[w][2]);

    return moves;
}
