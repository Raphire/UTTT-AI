#include "TTTAI.h"

#include <algorithm>

int TTTAI::RateMove(const AssessedState & assessedState, const Move & move)
{
    Board board = assessedState.state.subBoards[move.x / 3 + (move.y / 3) * 3];
    int subMove = move.x % 3 + (move.y % 3) * 3;

    std::vector<int> moves = TTT::GetMoves(board);

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

    for (auto & win : TTT::wins)
        if(board[win[0]] == p && board[win[1]] == p) moves.push_back(win[2]);
        else if(board[win[1]] == p && board[win[2]] == p) moves.push_back(win[0]);
        else if(board[win[0]] == p && board[win[2]] == p) moves.push_back(win[1]);

    return moves;
}

std::vector<int> TTTAI::GetSetupMoves(const Board &board, Player p)
{
    std::vector<int> moves;
    std::vector<int> cells = TTT::GetCellsOccupiedByPlayer(board, p);

    for (auto & win : TTT::wins)
    {
        if(board[win[0]] == p && board[win[1]] == Player::None && board[win[2]] == Player::None)
        {
            moves.push_back(win[1]);
            moves.push_back(win[2]);
        }

        if(board[win[1]] == p && board[win[0]] == Player::None && board[win[2]] == Player::None)
        {
            moves.push_back(win[0]);
            moves.push_back(win[2]);
        }

        if(board[win[2]] == p && board[win[1]] == Player::None && board[win[0]] == Player::None)
        {
            moves.push_back(win[1]);
            moves.push_back(win[0]);
        }
    }

    return moves;
}
