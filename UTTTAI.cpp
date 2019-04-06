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

int UTTTAI::RateByPosition(const Move & move, const State & state)
{
    int m = uttt::GetMicroMove(move);

    std::array<Player, 9> nextBoard = state.subBoards[(move.x % 3) + 3 * (move.y % 3)];

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

int UTTTAI::RateByMacrogameFieldValue(const Move & move, const State & state)
{
    std::array<int, 9> potentialWins = {{3, 2, 3, 2, 4, 2, 3, 2, 3}}; // basically monte carlo in a single line for ttt

    // Send opponent to macro field with least amount of potential wins
    int subMove = move.x % 3 + 3 * (move.y % 3);
    int score = -potentialWins[subMove];

    return score;
}

Move UTTTAI::FindBestMove(const State &state, const int &timeout)
{
    auto turnStartTime = std::chrono::steady_clock::now();
    long long int timeElapsed;

    std::cerr << state << std::endl;

    std::vector<SelectionStage> selectionStages;

    selectionStages.push_back({
       "MiniMaxAB",
       100,
       RateMovesWithMiniMaxAB
    });

    selectionStages.push_back({
        "TTT strategies",
        100,
        RateMovesWithTTTStrats
    });

    selectionStages.push_back({
        "Position rating",
        100,
        RateMovesWithPosition
    });

    selectionStages.push_back({
        "Next board position",
        100,
        RateMovesWithNextBoardPosition
    });

    std::vector<Move> moves = uttt::getMoves(state);
    std::vector<int> moveRatings;
    std::vector<Move> bestMoves = moves;

    std::cerr << "Starting assessment of current game-state: " << std::endl;

    std::vector<int> relevantMacroIndicesOffensive;
    std::vector<int> relevantMacroIndicesDefensive;

    Player potentialWinners = Player::None;

    std::array<Player, 9> potentialSubBoardWinners;
    for(int b = 0; b < state.subBoards.size(); b++)
        potentialSubBoardWinners[b] = ttt::IsWinnableForPlayer(state.subBoards[b]);

    for(int w = 0; w < 8; w++)
    {
        Player a = potentialSubBoardWinners[ttt::wins[w][0]];
        Player b = potentialSubBoardWinners[ttt::wins[w][1]];
        Player c = potentialSubBoardWinners[ttt::wins[w][2]];

        if(a == Player::None || b == Player::None || c == Player::None) continue; // a, b or c is never None after this point
        if(a == b && a == c && a == Player::Both) {
            potentialWinners = Player::Both;

            relevantMacroIndicesOffensive.push_back(ttt::wins[w][0]);
            relevantMacroIndicesOffensive.push_back(ttt::wins[w][1]);
            relevantMacroIndicesOffensive.push_back(ttt::wins[w][2]);

            relevantMacroIndicesDefensive.push_back(ttt::wins[w][0]);
            relevantMacroIndicesDefensive.push_back(ttt::wins[w][1]);
            relevantMacroIndicesDefensive.push_back(ttt::wins[w][2]);
        }
        else if((a == Player::X || a == Player::Both) && (b == Player::X || b == Player::Both) && (c == Player::X || c == Player::Both)) {
            if(state.player == Player::X) {
                relevantMacroIndicesOffensive.push_back(ttt::wins[w][0]);
                relevantMacroIndicesOffensive.push_back(ttt::wins[w][1]);
                relevantMacroIndicesOffensive.push_back(ttt::wins[w][2]);
            } else {
                relevantMacroIndicesDefensive.push_back(ttt::wins[w][0]);
                relevantMacroIndicesDefensive.push_back(ttt::wins[w][1]);
                relevantMacroIndicesDefensive.push_back(ttt::wins[w][2]);
            }

            if(potentialWinners == Player::O) potentialWinners = Player::Both;
            else if(potentialWinners != Player::Both) potentialWinners = Player::X;
        }
        else if((a == Player::O || a == Player::Both) && (b == Player::O || b == Player::Both) && (c == Player::O || c == Player::Both)) {
            if(state.player == Player::O) {
                relevantMacroIndicesOffensive.push_back(ttt::wins[w][0]);
                relevantMacroIndicesOffensive.push_back(ttt::wins[w][1]);
                relevantMacroIndicesOffensive.push_back(ttt::wins[w][2]);
            } else {
                relevantMacroIndicesDefensive.push_back(ttt::wins[w][0]);
                relevantMacroIndicesDefensive.push_back(ttt::wins[w][1]);
                relevantMacroIndicesDefensive.push_back(ttt::wins[w][2]);
            }

            if(potentialWinners == Player::X) potentialWinners = Player::Both;
            else if(potentialWinners != Player::Both) potentialWinners = Player::O;
        }
    }

    int numPossibleWins = relevantMacroIndicesOffensive.size() / 3;
    int numPossibleLosses = relevantMacroIndicesDefensive.size() / 3;

    std::array<int, 9> macroFieldWorthsOffensive;
    std::array<int, 9> macroFieldWorthsDefensive;

    for(int i = 0; i < 9; i++) macroFieldWorthsOffensive[i] = std::count(relevantMacroIndicesOffensive.begin(), relevantMacroIndicesOffensive.end(), i);
    for(int i = 0; i < 9; i++) macroFieldWorthsDefensive[i] = std::count(relevantMacroIndicesDefensive.begin(), relevantMacroIndicesDefensive.end(), i);

    std::cerr << "Detected " << numPossibleWins << " possible macro-win conditions." << std::endl;
    std::cerr << "Detected " << numPossibleLosses << " possible macro-loose conditions." << std::endl;

    std::cerr << "Macroboard offensive values: " << macroFieldWorthsOffensive;
    std::cerr << "Macroboard defensive values: " << macroFieldWorthsDefensive;


    if(potentialWinners == Player::Both) std::cerr << "Either player can technically still win this match, i should manually asses aggressiveness." << std::endl; // TODO: Compose function to assess balance of power + balanced strategy as function of BOP
    else if(potentialWinners == state.player) std::cerr << "Only this bot has a chance of winning, i should assume an offensive strategy." << std::endl; // TODO: Compose all-out offensive strategy
    else if(potentialWinners == state.opponent) std::cerr << "Only the opponent has a chance of winning, i should assume a defensive strategy." << std::endl; // TODO: Compose defensive strategy
    else {
        std::cerr << "Neither player can win this match, no reason to risk a timeout by dedicating cpu cycles to move selection." << std::endl;
        return moves[0];
    }

    // Edge cases...
    if (moves.empty()) std::cerr << "ERROR: Board appears to be full, yet AI is asked to pick a move!" << std::endl;
    if (moves.size() == 1) return moves[0]; // Might occur later in matches

    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::cerr << "Starting elimination of moves. (" << timeElapsed << " ms)" << std::endl;

    for(int s = 0; s < selectionStages.size(); s++)
    {
        moveRatings = selectionStages[s].evaluate(bestMoves, state);
        std::vector<Move> newBestMoves = PickValuesAtIndicesOfList(bestMoves, BestRatingIndicesOfList(moveRatings));

        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
        std::cerr << "Stage #" << s+1 << " (" << selectionStages[s].name << ") eliminated " << bestMoves.size() - newBestMoves.size() << " of " << bestMoves.size() << " moves. (" << timeElapsed << " ms)" << std::endl;
        std::cerr << "Highest rating moves are rated " << moveRatings[BestRatingIndicesOfList(moveRatings)[0]] << "." << std::endl << std::endl;

        if(newBestMoves.size() == 1)
        {
            std::cerr << "Found a single best move, search done." << std::endl;
            std::cerr << "---------------------------------------------------------------------------------------" << std::endl;
            return newBestMoves[0];
        }
        else bestMoves = newBestMoves;
    }

    /// STAGE RANDOM
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::cerr << "Multiple optimal moves have been found. Selecting one randomly... (" << timeElapsed << " ms) " << std::endl;
    std::cerr << "---------------------------------------------------------------------------------------" << std::endl;
    return *select_randomly(bestMoves.begin(), bestMoves.end());
}

std::vector<int> UTTTAI::RateMovesWithMiniMaxAB(const std::vector<Move> & moves, const State & state)
{
    if(state.round < 12) return std::vector<int>(moves.size()); // TODO: Guessed value (12)

    long long int timeElapsed;
    auto startTime = std::chrono::steady_clock::now();
    std::vector<int> ratings;
    int searchDepth = INITIAL_SEARCH_DEPTH;

    do {
        bool searchTreeExhausted = true;
        ratings.clear();
        for (int i = 0; i < moves.size(); i++)
        {
            bool fullMoveTreeEvaluated = true;
            State child = uttt::doMove(state, moves[i]);
            ratings.push_back(TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, -100, +100, &fullMoveTreeEvaluated));
            if(!fullMoveTreeEvaluated) searchTreeExhausted = false;
        }
        if(searchTreeExhausted) {
            std::cerr << "MiniMax traversed entire game-state tree." << std::endl;
            break;
        }
        searchDepth++; // Increase search depth for next iteration.
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
    }
    while (timeElapsed * moves.size() < state.time_per_move); // TODO: Loop assumes branching factor to remain constant while it doesn't

    std::cerr << "Searched until depth: " << searchDepth << std::endl;

    return ratings;
}

std::vector<int> UTTTAI::RateMovesWithTTTStrats(const std::vector<Move> & moves, const State & state)
{
    std::vector<int> ratings;

    for (int m = 0; m < moves.size(); m++)
    {
        ratings.push_back(TTTAI::RateMove(state, moves[m]));
    }

    return ratings;
}

std::vector<int> UTTTAI::RateMovesWithPosition(const std::vector<Move> &moves, const State &state)
{
    std::vector<int> ratings;

    for(int i = 0; i < moves.size(); i++)
        ratings.push_back(RateByPosition(moves[i], state));

    return ratings;
}

std::vector<int> UTTTAI::RateMovesWithNextBoardPosition(const std::vector<Move> &moves, const State &state)
{
    std::vector<int> ratings;

    for(int i = 0; i < moves.size(); i++)
        ratings.push_back(RateByMacrogameFieldValue(moves[i], state));

    return ratings;
}

Board UTTTAI::GetMacroBoardStripped(const State &state)
{
    Board board = state.macroboard;
    for(int i = 0; i < 9; i++)
        if(board[i] == Player::Active) board[i] == Player::None;

    return board;
}

Board UTTTAI::GetPotentialMacroBoard(const State &state)
{
    Board potentialMacroBoard = GetMacroBoardStripped(state);
    for(int s = 0; s < 9; s++)
        if(potentialMacroBoard[s] == Player::None)
            potentialMacroBoard[s] = ttt::IsWinnableForPlayer(state.subBoards[s]);

    return potentialMacroBoard;
}

Player UTTTAI::GetPotentialWinners(const State &state)
{
    Player potentialWinners = Player::None;

    std::array<Player, 9> potentialSubBoardWinners;
    for(int b = 0; b < state.subBoards.size(); b++)
        potentialSubBoardWinners[b] = ttt::IsWinnableForPlayer(state.subBoards[b]);

    for(int w = 0; w < 8; w++)
    {
        Player a = potentialSubBoardWinners[ttt::wins[w][0]];
        Player b = potentialSubBoardWinners[ttt::wins[w][1]];
        Player c = potentialSubBoardWinners[ttt::wins[w][2]];

        if(a == Player::None || b == Player::None || c == Player::None) continue; // a, b or c is never None after this point
        if(a == b && a == c && a == Player::Both) {
            potentialWinners = Player::Both;
            break;
        }
        else if((a == Player::X || a == Player::Both) && (b == Player::X || b == Player::Both) && (c == Player::X || c == Player::Both)) {
            if(potentialWinners == Player::O) {
                potentialWinners = Player::Both;
                break;
            } else potentialWinners = Player::X;
        }
        else if((a == Player::O || a == Player::Both) && (b == Player::O || b == Player::Both) && (c == Player::O || c == Player::Both)) {
            if(potentialWinners == Player::X) {
                potentialWinners = Player::Both;
                break;
            } else potentialWinners = Player::O;
        }
    }

    return potentialWinners;
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

