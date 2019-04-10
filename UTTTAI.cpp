//
// Created by Jorn on 04/04/2019.
//

#include <chrono>
#include <algorithm>
#include "UTTTAI.h"
#include "uttt.h"
#include "ttt.h"
#include "TTTAI.h"
#include "MiniMaxSearch.h"

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

    for(Move move:moves)
        children.push_back(uttt::doMove(state, move));

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
                            UTTTAI::RateMovesWithMiniMaxAB
                    },
                    SelectionStage {
                            "Macro field selection",
                            100,
                            UTTTAI::RateMovesByMacroRelevance
                    },
                    SelectionStage {
                            "TTT strategies",
                            100,
                            UTTTAI::RateMovesWithTTTStrats
                    },
                    SelectionStage {
                            "Position rating",
                            100,
                            UTTTAI::RateMovesWithPosition
                    },
                    SelectionStage {
                            "Next board position",
                            100,
                            UTTTAI::RateMovesWithNextBoardPosition
                    }
            };

    std::vector<Move> moves = uttt::getMoves(state);
    std::vector<int> moveRatings;
    std::vector<Move> bestMoves = moves;

    std::cerr << "Assessing current state..." << std::endl;
    AssessedState assessedState = AssessState(state);
    if(assessedState.potentialWinners == Player::Both) std::cerr << "Either player can technically still win this match, i should manually assess aggressiveness." << std::endl; // TODO: Compose function to assess balance of power + balanced strategy as function of BOP
    else if(assessedState.potentialWinners == state.player) std::cerr << "Only this bot has a chance of winning, i should assume an offensive strategy." << std::endl; // TODO: Compose all-out offensive strategy
    else if(assessedState.potentialWinners == state.opponent) std::cerr << "Only the opponent has a chance of winning, i should assume a defensive strategy." << std::endl; // TODO: Compose defensive strategy
    else std::cerr << "Neither player can win this match, no reason to risk a timeout by dedicating cpu cycles to move selection." << std::endl;
    std::cerr << "-----------------------------------------------------------------" << std::endl;

    // Edge cases...
    if (moves.empty()) std::cerr << "ERROR: Board appears to be full, yet AI is asked to pick a move!" << std::endl;
    if (moves.size() == 1) return moves[0]; // Might occur later in matches

    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::cerr << "Starting elimination of moves. (" << timeElapsed << " ms)" << std::endl;

    for(int s = 0; s < selectionStages.size(); s++)
    {
        moveRatings = selectionStages[s].evaluate(bestMoves, assessedState);
        std::vector<Move> newBestMoves = PickValuesAtIndicesOfList(bestMoves, BestRatingIndicesOfList(moveRatings));

        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
        std::cerr << "Stage #" << s+1 << " (" << selectionStages[s].name << ") eliminated " << bestMoves.size() - newBestMoves.size() << " of " << bestMoves.size() << " moves. (" << timeElapsed << " ms)" << std::endl;
        std::cerr << "Highest rating moves are rated " << moveRatings[BestRatingIndicesOfList(moveRatings)[0]] << "." << std::endl << std::endl;

        if(newBestMoves.size() == 1)
        {
            std::cerr << "Found a single best move, search done." << std::endl;
            std::cerr << "---------------------------------------------------------------------------------------" << std::endl;
            std::cerr << "---------------------------------------------------------------------------------------" << std::endl;
            return newBestMoves[0];
        }
        else bestMoves = newBestMoves;
    }

    /// STAGE RANDOM
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::cerr << "Multiple optimal moves have been found. Selecting one randomly... (" << timeElapsed << " ms) " << std::endl;
    std::cerr << "---------------------------------------------------------------------------------------" << std::endl;
    std::cerr << "---------------------------------------------------------------------------------------" << std::endl;

    return *select_randomly(bestMoves.begin(), bestMoves.end());
}

std::vector<int> UTTTAI::RateMovesWithMiniMaxAB(const std::vector<Move> & moves, const AssessedState & assessedState)
{
    if(assessedState.state.round < 12) return std::vector<int>(moves.size()); // TODO: Guessed value (12)

    auto startTime = std::chrono::steady_clock::now();

    MiniMaxSearch<State> search = MiniMaxSearch<State>(EvaluateState, GetChildStates);
    std::vector<int> ratings = search.evaluateBranchConcurrent(assessedState.state);

    return ratings;
}

std::vector<int> UTTTAI::RateMovesWithTTTStrats(const std::vector<Move> & moves, const AssessedState & assessedState)
{
    std::vector<int> ratings;

    for (int m = 0; m < moves.size(); m++)
        ratings.push_back(TTTAI::RateMove(assessedState, moves[m]));

    return ratings;
}

std::vector<int> UTTTAI::RateMovesWithPosition(const std::vector<Move> &moves, const AssessedState &assessedState)
{
    std::vector<int> ratings;

    for(int i = 0; i < moves.size(); i++)
        ratings.push_back(RateByPosition(moves[i], assessedState));

    return ratings;
}

std::vector<int> UTTTAI::RateMovesWithNextBoardPosition(const std::vector<Move> &moves, const AssessedState &assessedState)
{
    std::vector<int> ratings;

    for(int i = 0; i < moves.size(); i++) {
        std::array<Player, 9> nextSubBoard;

        int macroMove = moves[i].x / 3 + 3 * (moves[i].y / 3);
        int nextMacroBoardIndex = moves[i].x % 3 + 3 * (moves[i].y % 3);
        if(macroMove == nextMacroBoardIndex)
            nextSubBoard = ttt::DoMove(assessedState.state.subBoards[nextMacroBoardIndex], nextMacroBoardIndex, assessedState.state.player);
        else
            nextSubBoard = assessedState.state.subBoards[nextMacroBoardIndex];

        if(ttt::GetMoves(nextSubBoard).size() == 0) // Next player gets to choose which macro-field to play on...
            ratings.push_back(-20); // Allowing opponent to pick is a bad choice, and should only be done when there's no other option (At this point in elimination)
        else // Preferably send the opponent to the least important macro-field
        {
            // A field that rates higher for defensive/offensive relevance will rate lower
            int rating = 0;
            rating -= assessedState.macroFieldWorthsDefensive[nextMacroBoardIndex];
            rating -= assessedState.macroFieldWorthsOffensive[nextMacroBoardIndex];
            ratings.push_back(rating);
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



