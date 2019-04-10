//
// Created by Jorn on 04/04/2019.
//

#include <chrono>
#include <algorithm>

#include "UTTTAI.h"
#include "uttt.h"
#include "ttt.h"
#include "TreeSearch.h"
#include "TTTAI.h"
#include "RiddlesIOLogger.h"

int UTTTAI::EvaluateState(const State & state)
{
    Player winner = state.winner;                           // Is there a winner?
    if (winner == state.player) return +1;						// Bot has won in evaluated state
    if (winner == Player::None) return 0;						// No winner, rate state with heuristics
    return -1;                                               // Opponent has won in evaluated state
}

std::vector<State> UTTTAI::GetChildStates(const State &state)
{
    std::vector<State> children;
    std::vector<Move> moves = uttt::getMoves(state);

    for (Move m : moves)
        children.push_back(uttt::doMove(state, m));

    return children;
}

int UTTTAI::RateByPosition(const Move & move, const AssessedState & assessedState)
{
    std::array<Player, 9> nextBoard = assessedState.state.subBoards[(move.x % 3) + 3 * (move.y % 3)];

    auto nextMoves = ttt::GetMoves(nextBoard);
    if(nextMoves.size() == 0) return -1; // Making this move gives the opponent the most options, as he gets the choice which micro board to play on
    Player nextWinnableBy = ttt::IsWinnableForPlayer(nextBoard);

    // This board can still be won by both players, it is still of good use
    if(nextWinnableBy == Player::Both) return 0;

    // Someone can still win on this board, but sending the opponent here would be better than previous options
    if(nextWinnableBy == Player::X || nextWinnableBy == Player::O) return 1;

    // It would be ideal to force an opponent to move here, as this board is not of any use to anyone
    if(nextWinnableBy == Player::None) return 2;
}

Move UTTTAI::FindBestMove(const State &state, const int &timeout)
{
    auto turnStartTime = std::chrono::steady_clock::now();
    long long int timeElapsed;

    std::array<SelectionStage, 5> selectionStages =
            {
                    SelectionStage {
                        "MiniMaxAB",
                        100,
                        UTTTAI::RateMovesByMiniMaxAB
                    },
                    SelectionStage {
                        "Macro field selection",
                        100,
                        UTTTAI::RateMovesByMacroRelevance
                    },
                    SelectionStage {
                        "TTT strategies",
                        100,
                        UTTTAI::RateMovesByTTTStrats
                    },
                    SelectionStage {
                        "Position rating",
                        100,
                        UTTTAI::RateMovesByPosition
                    },
                    SelectionStage {
                        "Next board position",
                        100,
                        UTTTAI::RateMovesByNextBoardPosition
                    }
            };

    std::vector<Move> moves = uttt::getMoves(state);
    std::vector<int> moveRatings;
    std::vector<Move> bestMoves = moves;

    AssessedState assessedState = AssessState(state);

    // Edge cases...
    if (moves.empty()) RiddlesIOLogger::Log(ERROR_DO_MOVE_ON_FINISHED_GAME, {});
    if (moves.size() == 1) return moves[0];

    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    RiddlesIOLogger::Log(BEGIN_ELIMINATION_OF_MOVES, {std::to_string(timeElapsed)});

    for(int s = 0; s < selectionStages.size(); s++)
    {
        moveRatings = selectionStages[s].evaluate(bestMoves, assessedState);
        std::vector<Move> newBestMoves = PickValuesAtIndicesOfList(bestMoves, BestRatingIndicesOfList(moveRatings));
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();

        RiddlesIOLogger::Log(SELECTIONSTAGE_PASSED_SUMMARY,{std::to_string(s+1),selectionStages[s].name,std::to_string(bestMoves.size() - newBestMoves.size()),std::to_string(bestMoves.size()),std::to_string(timeElapsed),std::to_string(moveRatings[BestRatingIndicesOfList(moveRatings)[0]]),RiddlesIOLogger::MovesToString(newBestMoves)});

        if(newBestMoves.size() == 1)
        {
            RiddlesIOLogger::Log(SINGLE_BEST_MOVE_FOUND, {RiddlesIOLogger::MovesToString({newBestMoves[0]})});
            return newBestMoves[0];
        }
        bestMoves = newBestMoves;
    }

    /// STAGE RANDOM
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    RiddlesIOLogger::Log(MULTIPLE_BEST_MOVES_FOUND, {std::to_string(timeElapsed), RiddlesIOLogger::MovesToString(bestMoves)});

    // TODO: This doesnt appear to select a random move from list, it just picks the first one...
    Move selected = *select_randomly(bestMoves.begin(), bestMoves.end());
    return selected;
}

std::vector<int> UTTTAI::RateMovesByMiniMaxAB(const std::vector<Move> &moves, const AssessedState &state)
{
    if(state.state.round < 12) return std::vector<int>(moves.size()); // TODO: Guessed value (12)

    long long int timeElapsed;
    bool searchTreeExhausted;
    auto startTime = std::chrono::steady_clock::now();
    std::vector<int> ratings;
    int searchDepth = INITIAL_SEARCH_DEPTH;

    do {
        searchTreeExhausted = true;
        ratings.clear();
        for (int i = 0; i < moves.size(); i++)
        {
            bool fullMoveTreeEvaluated = true;
            State child = uttt::doMove(state.state, moves[i]);
            ratings.push_back(TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, -100, +100, &fullMoveTreeEvaluated));
            if(!fullMoveTreeEvaluated) searchTreeExhausted = false;
        }
        if(searchTreeExhausted) break;
        searchDepth++; // Increase search depth for next iteration.
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
    }
    while (timeElapsed * (56-state.state.round) < state.state.time_per_move); // TODO: Loop assumes branching factor to remain constant while it doesn't

    if(searchTreeExhausted) RiddlesIOLogger::Log(MINIMAX_SEARCH_FINISHED_ALL_EVALUATED, {});
    else RiddlesIOLogger::Log(MINIMAX_SEARCH_FINISHED, {std::to_string(searchDepth)});

    return ratings;
}

std::vector<int> UTTTAI::RateMovesByTTTStrats(const std::vector<Move> &moves, const AssessedState &state)
{
    std::vector<int> ratings;

    for (int m = 0; m < moves.size(); m++)
        ratings.push_back(TTTAI::RateMove(state, moves[m]));

    return ratings;
}

std::vector<int> UTTTAI::RateMovesByPosition(const std::vector<Move> &moves, const AssessedState &state)
{
    std::vector<int> ratings;

    for(int i = 0; i < moves.size(); i++)
        ratings.push_back(RateByPosition(moves[i], state));

    return ratings;
}

std::vector<int> UTTTAI::RateMovesByNextBoardPosition(const std::vector<Move> &moves, const AssessedState &state)
{
    std::vector<int> ratings;

    for(int i = 0; i < moves.size(); i++) {
        std::array<Player, 9> nextSubBoard;

        int macroMove = moves[i].x / 3 + 3 * (moves[i].y / 3);
        int nextMacroBoardIndex = moves[i].x % 3 + 3 * (moves[i].y % 3);
        if(macroMove == nextMacroBoardIndex)
            nextSubBoard = ttt::DoMove(state.state.subBoards[nextMacroBoardIndex], nextMacroBoardIndex, state.state.player);
        else
            nextSubBoard = state.state.subBoards[nextMacroBoardIndex];

        if(ttt::GetMoves(nextSubBoard).size() == 0) // Next player gets to choose which macro-field to play on...
            ratings.push_back(-20); // Allowing opponent to pick is a bad choice, and should only be done when there's no other option (At this point in elimination)
        else // Preferably send the opponent to the least important macro-field, with the lowest winning chance
        {
            // A field that rates higher for defensive/offensive relevance will rate lower
            int rating = 0;
            rating -= state.macroFieldWorthsDefensive[nextMacroBoardIndex];
            rating -= state.macroFieldWorthsOffensive[nextMacroBoardIndex];
            ratings.push_back(rating);
        }
    }

    std::vector<int> bestMoves = BestRatingIndicesOfList(ratings);
    if(bestMoves.size() > 1 && bestMoves[0] != -20) // There's still moves that rate equally, try ranking them
    {
        // Moves making opponent play on a sub-board where he has a lower chance of winning rate higher
        for(int i = 0; i < bestMoves.size(); i++)
        {
            int nextMacroBoardIndex = moves[bestMoves[i]].x % 3 + 3 * (moves[bestMoves[i]].y % 3);

            ratings[bestMoves[i]] += state.minMovesToPartialLosses[nextMacroBoardIndex];
            ratings[bestMoves[i]] -= state.minMovesToPartialWins[nextMacroBoardIndex];
        }
    }

    return ratings;
}

Board UTTTAI::GetMacroBoardStripped(const State &state)
{
    Board board = state.macroboard;
    for(int i = 0; i < 9; i++)
        if(board[i] == Player::Active) board[i] == Player::None;

    return board;
}

AssessedState UTTTAI::AssessState(const State &state)
{
    AssessedState assessedState;
    assessedState.state = state;

    for(int b = 0; b < 9; b++) assessedState.minMovesToPartialWins[b] = ttt::GetMinimumMovesToWin(state.subBoards[b], state.player);
    for(int b = 0; b < 9; b++) assessedState.minMovesToPartialLosses[b] = ttt::GetMinimumMovesToWin(state.subBoards[b], state.opponent);

    // Find out whether or not each sub-board can be won by which player(s)
    for(int b = 0; b < state.subBoards.size(); b++) assessedState.potentialSubBoardWinners[b] = ttt::IsWinnableForPlayer(state.subBoards[b]);

    // Add indices of relevant macro-boards to list for every time it represents a win.
    // This will help us find out whether we want to send our opponent to such board.
    for(int w = 0; w < 8; w++)
    {
        Player a = assessedState.potentialSubBoardWinners[ttt::wins[w][0]];
        Player b = assessedState.potentialSubBoardWinners[ttt::wins[w][1]];
        Player c = assessedState.potentialSubBoardWinners[ttt::wins[w][2]];

        if(a == Player::None || b == Player::None || c == Player::None) continue; // a, b or c is never None after this point
        if(a == b && a == c && a == Player::Both) {
            assessedState.potentialWinners = Player::Both;

            assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][0]);
            assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][1]);
            assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][2]);

            assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][0]);
            assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][1]);
            assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][2]);
        }
        else if((a == Player::X || a == Player::Both) && (b == Player::X || b == Player::Both) && (c == Player::X || c == Player::Both)) {
            if(state.player == Player::X) {
                assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][0]);
                assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][1]);
                assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][2]);
            } else {
                assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][0]);
                assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][1]);
                assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][2]);
            }

            if(assessedState.potentialWinners == Player::O) assessedState.potentialWinners = Player::Both;
            else if(assessedState.potentialWinners != Player::Both) assessedState.potentialWinners = Player::X;
        }
        else if((a == Player::O || a == Player::Both) && (b == Player::O || b == Player::Both) && (c == Player::O || c == Player::Both)) {
            if(state.player == Player::O) {
                assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][0]);
                assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][1]);
                assessedState.relevantMacroIndicesOffensive.push_back(ttt::wins[w][2]);
            } else {
                assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][0]);
                assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][1]);
                assessedState.relevantMacroIndicesDefensive.push_back(ttt::wins[w][2]);
            }

            if(assessedState.potentialWinners == Player::X) assessedState.potentialWinners = Player::Both;
            else if(assessedState.potentialWinners != Player::Both) assessedState.potentialWinners = Player::O;
        }
    }

    // Assess the offensive and defensive value of each macro-board, this can help us decide which
    // macro-board  we want to win first, or which one we do not want our opponent to win anytime soon...
    for(int i = 0; i < 9; i++) assessedState.macroFieldWorthsOffensive[i] = std::count(assessedState.relevantMacroIndicesOffensive.begin(), assessedState.relevantMacroIndicesOffensive.end(), i);
    for(int i = 0; i < 9; i++) assessedState.macroFieldWorthsDefensive[i] = std::count(assessedState.relevantMacroIndicesDefensive.begin(), assessedState.relevantMacroIndicesDefensive.end(), i);

    // Assess the minimum amount of moves each player has to make to reach their respective win conditions.
    for(int w = 0; w < 8; w++)
    {
        if(std::find(assessedState.relevantMacroIndicesOffensive.begin(), assessedState.relevantMacroIndicesOffensive.end(), ttt::wins[w][0]) != assessedState.relevantMacroIndicesOffensive.end()
           && std::find(assessedState.relevantMacroIndicesOffensive.begin(), assessedState.relevantMacroIndicesOffensive.end(), ttt::wins[w][1]) != assessedState.relevantMacroIndicesOffensive.end()
           && std::find(assessedState.relevantMacroIndicesOffensive.begin(), assessedState.relevantMacroIndicesOffensive.end(), ttt::wins[w][2]) != assessedState.relevantMacroIndicesOffensive.end())
            assessedState.minMovesToWin[w] = assessedState.minMovesToPartialWins[ttt::wins[w][0]] + assessedState.minMovesToPartialWins[ttt::wins[w][1]] + assessedState.minMovesToPartialWins[ttt::wins[w][2]];
        else
            assessedState.minMovesToWin[w] = 0;

        if(std::find(assessedState.relevantMacroIndicesDefensive.begin(), assessedState.relevantMacroIndicesDefensive.end(), ttt::wins[w][0]) != assessedState.relevantMacroIndicesDefensive.end()
           && std::find(assessedState.relevantMacroIndicesDefensive.begin(), assessedState.relevantMacroIndicesDefensive.end(), ttt::wins[w][1]) != assessedState.relevantMacroIndicesDefensive.end()
           && std::find(assessedState.relevantMacroIndicesDefensive.begin(), assessedState.relevantMacroIndicesDefensive.end(), ttt::wins[w][2]) != assessedState.relevantMacroIndicesDefensive.end())
            assessedState.minMovesToLoose[w] = assessedState.minMovesToPartialLosses[ttt::wins[w][0]] + assessedState.minMovesToPartialLosses[ttt::wins[w][1]] + assessedState.minMovesToPartialLosses[ttt::wins[w][2]];
        else
            assessedState.minMovesToLoose[w] = 0;
    }

    return assessedState;
}

std::vector<int> UTTTAI::RateMovesByMacroRelevance(const std::vector<Move> & moves, const AssessedState & assessedState)
{
    std::vector<int> ratings;
    for(int m = 0; m < moves.size(); m++)
    {
        int macroBoardIndex = moves[m].x / 3 + 3 * (moves[m].y / 3);
        ratings.push_back(assessedState.macroFieldWorthsOffensive[macroBoardIndex] + assessedState.macroFieldWorthsDefensive[macroBoardIndex]);
    }
    return ratings;
}

std::vector<int> BestRatingIndicesOfList(const std::vector<int> &vals)
{
    int bestVal = vals[0];
    std::vector<int> indicesOfHighestValues;

    for (int i = 0; i < vals.size(); i++)
        if(vals[i] > bestVal) bestVal = vals[i];

    for (int i = 0; i < vals.size(); i++)
        if(vals[i] == bestVal) indicesOfHighestValues.push_back(i);

    return indicesOfHighestValues;
}

template<class O>
std::vector<O> PickValuesAtIndicesOfList(const std::vector<O> &list, const std::vector<int> &indices)
{
    std::vector<O> data;
    for(int i = 0; i < indices.size(); i++)
        data.push_back(list[indices[i]]);
    return data;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Move> &m)
{
    if(m.size() == 0) return os;

    os << "(X: " << m[0].x << " Y: " << m[0].y << ")";

    for(int mi = 1; mi < m.size(); mi++)
        os << ", (X: " << m[mi].x << " Y: " << m[mi].y << ")";

    return os;
}




