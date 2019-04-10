//
// Created by Jorn on 04/04/2019.
//

#include <algorithm>

#include "TTTAI.h"
#include "ttt.h"

/// The following ratings only apply to SUB-games and therefore should
/// only be used to rank different moves within the SAME sub-boards.
int TTTAI::RateMove(const AssessedState & assessedState, const Move & move)
{
    Board board = assessedState.state.subBoards[move.x / 3 + (move.y / 3) * 3];
    int subMove = move.x % 3 + (move.y % 3) * 3;

    std::vector<int> moves = ttt::GetMoves(board);

    std::vector<int> winningMoves = GetWinningMoves(board, assessedState.state.player);
    std::vector<int> defendingMoves = GetWinningMoves(board, assessedState.state.opponent);

    std::vector<int> playerSetupMoves = GetSetupMoves(board, assessedState.state.player);
    std::vector<int> otherSetupMoves = GetSetupMoves(board, assessedState.state.opponent);

    int score = 0;

    // Attacking moves are rated higher than defending moves as you already implicitly defend by winning a board.
    if(std::find(winningMoves.begin(), winningMoves.end(), subMove) != winningMoves.end()) score = 100;

    if(winningMoves.empty())
    {
        // The more potential wins this move sets up, the better it will be rated
        score += std::count(playerSetupMoves.begin(), playerSetupMoves.end(), subMove);
        // The more potential setups of the opponent this move blocks the better
        score += std::count(otherSetupMoves.begin(), otherSetupMoves.end(), subMove);
        // Blocking opponent wins is a priority, doing so reaps a higher score
        score += 2 * std::count(defendingMoves.begin(), defendingMoves.end(), subMove);
    }

    return score;
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

    for(int w = 0; w < 8; w++) {
        int pi = -1;
        if(board[ttt::wins[w][0]] == p && board[ttt::wins[w][1]] == Player::None && board[ttt::wins[w][2]] == Player::None)
        {
            moves.push_back(ttt::wins[w][1]);
            moves.push_back(ttt::wins[w][2]);
        }

        if(board[ttt::wins[w][1]] == p && board[ttt::wins[w][0]] == Player::None && board[ttt::wins[w][2]] == Player::None)
        {
            moves.push_back(ttt::wins[w][0]);
            moves.push_back(ttt::wins[w][2]);
        }

        if(board[ttt::wins[w][2]] == p && board[ttt::wins[w][1]] == Player::None && board[ttt::wins[w][0]] == Player::None)
        {
            moves.push_back(ttt::wins[w][1]);
            moves.push_back(ttt::wins[w][0]);
        }
    }

    return moves;
}
