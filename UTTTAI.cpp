//
// Created by Jorn on 04/04/2019.
//

#include <chrono>
#include "UTTTAI.h"
#include "uttt.h"
#include "ttt.h"
#include "TreeSearch.h"
#include "TTTAI.h"

int UTTTAI::EvaluateState(const State & state, const Player & positive)
{
    Player winner = uttt::getWinner(state);                           // Is there a winner?
    if (winner == positive) return +1;						// Bot has won in evaluated state
    if (winner == Player::None) return 0;						// No winner, rate state with heuristics
    return -1;                                               // Opponent has won in evaluated state
}

std::vector<State> UTTTAI::GetChildStates(const State &state)
{
    std::vector<State> children;
    std::vector<Move> moves = uttt::getMoves(state);
    for (Move m : moves) children.push_back(uttt::doMove(state, m));
    return children;
}

int UTTTAI::RateByPosition(const Move & move, const std::array<std::array<Player, 9>, 9> & b)
{
    int m = uttt::GetMicroMove(move);

    std::array<Player, 9> nextBoard = uttt::GetNextSubBoard(b, move.x, move.y);

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

int UTTTAI::RateByMacrogameFieldValue(Move &move, std::array<std::array<Player, 9>, 9> array)
{
    std::array<int, 9> potentialWins = {{3, 2, 3, 2, 4, 2, 3, 2, 3}}; // basically monte carlo in a single line for ttt

    // Send opponent to macro field with least amount of potential wins
    int microMove = uttt::GetMicroMove(move);
    int score = -potentialWins[microMove];

    return score;
}

Move UTTTAI::FindBestMove(const State &state, const int &timeout)
{
    auto turnStartTime = std::chrono::steady_clock::now();
    long long int timeElapsed;
    const std::array<Board, 9> subBoards = ParseSubBoards(state.board);

    // Find all moves and rate them
    Player me = uttt::getCurrentPlayer(state);
    std::vector<Move> moves = uttt::getMoves(state);

    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Starting move search for round: " << round << " as Player " <<  me << "." << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

    // Edge cases...
    if (moves.empty()) std::cerr << "ERROR: Board appears to be full, yet AI is asked to pick a move!" << std::endl;
    if (moves.size() == 1) return moves[0]; // Might occur later in matches
    int searchDepth = INITIAL_SEARCH_DEPTH;


    /// STAGE ONE
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::vector<int> primaryRatings;

    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Stage #1 (Elimination): Evaluating a total of " << moves.size() << " moves using MinMaxAB. (" << timeElapsed << " ms)" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

    // TODO: This stage can probably be optimized a lot by rewriting the GetChildStates and EvaluateState functions, increasing search depth which will in its turn dramatically increase AI performance

    do {
        std::cerr << "Starting pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << ": Search depth of " << searchDepth << "." << std::endl;
        primaryRatings = std::vector<int>();

        bool searchTreeExhausted = true;
        for (int i = 0; i < moves.size(); i++)
        {
            bool fullMoveTreeEvaluated = true;
            State child = uttt::doMove(state, moves[i]);
            primaryRatings.push_back(TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, me, -1, +1, &fullMoveTreeEvaluated));

            if (primaryRatings[i] == +1)
            {
                std::cerr << "Found a route to a guaranteed win... Breaking off search!" << std::endl;
                return moves[i];
            }

            if (!fullMoveTreeEvaluated) searchTreeExhausted = false;
            else std::cerr << "Exhausted search tree of move #" << i << "." << std::endl;
        }
        if (searchTreeExhausted)
        {
            std::cerr << "Entire search tree was exhausted! Bot knows how this game will end if played perfectly by both sides." << std::endl;
            break;
        }
        searchDepth++; // Increase search depth for next iteration.

        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    }
    while ( // Keep searching 1 level deeper if there's enough time left, do not risk loosing time-bank time during first 2 rounds, its not worth it
            (timeElapsed * 3 < timeout && timeout > (5 * timeout))
            || // Game has a branching factor of 7, expect the time elapsed each iteration to be multiplied with this factor in worst case.
            (timeElapsed * 6 < timeout)
            );

    std::vector<Move> bestMoves = PickValuesAtIndicesOfList(moves, BestRatingIndicesOfList(primaryRatings));

    std::cerr << "Stage #1 eliminated " << moves.size() - bestMoves.size() << " of " << moves.size() << " moves." << std::endl;
    std::cerr << "Highest rating moves are rated " << primaryRatings[BestRatingIndicesOfList(primaryRatings)[0]] << "." << std::endl << std::endl;

    if(bestMoves.size() == 1) {
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
        std::cerr << "Found a single best move. Breaking of search! (" << timeElapsed << " ms) " << std::endl;
        return bestMoves[0];
    }


    /// STAGE TWO
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::vector<int> secondaryRatings = std::vector<int>();
    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Stage #2 (Elimination): Evaluating a total of " << bestMoves.size() << " moves using MinMaxAB for sub-games. (" << timeElapsed << " ms)" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

    std::array<std::array<Player, 9>, 9> microBoards = uttt::GetSubBoards(state.board);

    for (int m = 0; m < bestMoves.size(); m++)
    {
        int subBoard = bestMoves[m].x / 3 + (bestMoves[m].y / 3) * 3;
        int posInSubBoard = bestMoves[m].x % 3 + (bestMoves[m].y % 3) * 3;

        std::array<int, 9> scores = TTTAI::RateMoves(microBoards[subBoard], me);
        secondaryRatings.push_back(scores[posInSubBoard]);
    }
    std::vector<Move> bestMovesSecondary = PickValuesAtIndicesOfList(bestMoves, BestRatingIndicesOfList(secondaryRatings));
    std::cerr << "Stage #2 eliminated " << bestMoves.size() - bestMovesSecondary.size() << " of " << bestMoves.size() << " moves." << std::endl;
    std::cerr << "Highest rating moves are rated " << secondaryRatings[BestRatingIndicesOfList(secondaryRatings)[0]] << "." << std::endl << std::endl;

    if(bestMovesSecondary.size() == 1)
    {
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
        std::cerr << "Found a single best move. Breaking of search! (" << timeElapsed << " ms) " << std::endl;
        return bestMovesSecondary[0];
    }


    /// STAGE THREE
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::vector<int> tertiaryRatings = std::vector<int>();

    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Stage #3 (Elimination): Evaluating a total of " << bestMovesSecondary.size() << " moves using position rating. (" << timeElapsed << " ms)" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

    for(int i = 0; i < bestMovesSecondary.size(); i++)
        tertiaryRatings.push_back(RateByPosition(bestMovesSecondary[i], state.board));

    std::vector<Move> bestMovesTertiary = PickValuesAtIndicesOfList(bestMovesSecondary, BestRatingIndicesOfList(tertiaryRatings));
    std::cerr << "Stage #3 eliminated " << bestMovesSecondary.size() - bestMovesTertiary.size() << " of " << bestMovesSecondary.size() << " moves." << std::endl;
    std::cerr << "Highest rating moves are rated " << tertiaryRatings[BestRatingIndicesOfList(tertiaryRatings)[0]] << "." << std::endl << std::endl;

    if(bestMovesTertiary.size() == 1)
    {
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
        std::cerr << "Found a single best move. Breaking of search! (" << timeElapsed << " ms) " << std::endl;
        return bestMovesTertiary[0];
    }


    /// STAGE FOUR
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::vector<int> quaternaryRatings = std::vector<int>();

    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Stage #4 (Elimination): Evaluating a total of " << bestMovesTertiary.size() << " moves using position rating. (" << timeElapsed << " ms)" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

    for(int i = 0; i < bestMovesTertiary.size(); i++)
        quaternaryRatings.push_back(RateByMacrogameFieldValue(bestMovesTertiary[i], state.board));

    std::vector<Move> bestMovesQuaternary = PickValuesAtIndicesOfList(bestMovesTertiary, BestRatingIndicesOfList(quaternaryRatings));
    std::cerr << "Stage #4 eliminated " << bestMovesTertiary.size() - bestMovesQuaternary.size() << " of " << bestMovesTertiary.size() << " moves." << std::endl;
    std::cerr << "Highest rating moves are rated " << quaternaryRatings[BestRatingIndicesOfList(quaternaryRatings)[0]] << "." << std::endl << std::endl;

    if(bestMovesQuaternary.size() == 1)
    {
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
        std::cerr << "Found a single best move. Breaking of search! (" << timeElapsed << " ms) " << std::endl;
        return bestMovesQuaternary[0];
    }


    /// STAGE RANDOM
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::cerr << "Multiple optimal moves have been found. Selecting one randomly... (" << timeElapsed << " ms) " << std::endl;
    return *select_randomly(bestMovesQuaternary.begin(), bestMovesQuaternary.end());
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

static std::array<std::array<Player, 9>, 9> ParseSubBoards(const std::array<std::array<Player, 9>, 9> &state)
{
    std::array<std::array<Player, 9>, 9> subBoards;

    for(int x = 0; x < 3; x++)
        for(int y = 0; y < 3; y++)
            subBoards[y + x * 3] = uttt::GetSubBoard(state, x, y);
}

