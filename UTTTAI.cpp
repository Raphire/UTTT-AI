#include "UTTTAI.h"

#include <chrono>

#include "RiddlesIOLogger.h"
#include "MiniMaxSearch.h"
#include "TTTAI.h"


int UTTTAI::EvaluateState(const State & state)
{
    Player winner = state.winner;           // Is there a winner?
    if (winner == state.player) return +1;	// Bot has won in evaluated state
    if (winner == Player::None) return 0;	// No winner, rate state with heuristics
    return -1;                              // Opponent has won in evaluated state
}

std::vector<State> UTTTAI::GetChildStates(const State &state)
{
    std::vector<State> children;
    std::vector<Move> moves = UTTT::getMoves(state);

    for (Move m : moves)
        children.push_back(UTTT::doMove(state, m));

    return children;
}

int UTTTAI::RateByPosition(const Move & move, const AssessedState & assessedState)
{
    std::array<Player, 9> nextBoard = assessedState.state.subBoards[(move.x % 3) + 3 * (move.y % 3)];

    auto nextMoves = TTT::GetMoves(nextBoard);
    if(nextMoves.empty()) return -1; // Making this move gives the opponent the most options, as he gets the choice which micro board to play on
    Player nextWinnableBy = TTT::IsWinnableForPlayer(nextBoard);

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
                        UTTTAI::RateMovesByMiniMaxAB
                    },
                    SelectionStage {
                        "Macro field selection",
                        UTTTAI::RateMovesByMacroRelevance
                    },
                    SelectionStage {
                        "TTT strategies",
                        UTTTAI::RateMovesByTTTStrats
                    },
                    SelectionStage {
                        "Position rating",
                        UTTTAI::RateMovesByPosition
                    },
                    SelectionStage {
                        "Next board position",
                        UTTTAI::RateMovesByNextBoardPosition
                    }
            };

    std::vector<Move> moves = UTTT::getMoves(state);
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

    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    RiddlesIOLogger::Log(MULTIPLE_BEST_MOVES_FOUND, {std::to_string(timeElapsed), RiddlesIOLogger::MovesToString(bestMoves)});

    return bestMoves[0];
}

std::vector<int> UTTTAI::RateMovesByMiniMaxAB(const std::vector<Move> &moves, const AssessedState &state)
{
    if(state.state.round < 9) return std::vector<int>(moves.size());

    auto startTime = std::chrono::steady_clock::now();
    long long int timeElapsed;

    std::vector<int> ratings;

    MiniMaxSearch<State> mms = MiniMaxSearch<State>(EvaluateState, GetChildStates);

    ratings = mms.evaluateBranchUntilTimeout(state.state, state.state.time_per_move);

    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();

    if(mms.getLastSearchFullyEvaluated())
        RiddlesIOLogger::Log(MINIMAX_SEARCH_FINISHED_ALL_EVALUATED, {std::to_string(mms.getLastSearchNumNodesTraversed()), std::to_string(((double) mms.getLastSearchNumNodesTraversed()) / timeElapsed)});
    else
        RiddlesIOLogger::Log(MINIMAX_SEARCH_FINISHED, {std::to_string(mms.getLastSearchNumNodesTraversed()), std::to_string(((double) mms.getLastSearchNumNodesTraversed()) / timeElapsed)});

    return ratings;
}

std::vector<int> UTTTAI::RateMovesByTTTStrats(const std::vector<Move> &moves, const AssessedState &state)
{
    std::vector<int> ratings;

    for (auto move : moves)
        ratings.push_back(TTTAI::RateMove(state, move));

    return ratings;
}

std::vector<int> UTTTAI::RateMovesByPosition(const std::vector<Move> &moves, const AssessedState &state)
{
    std::vector<int> ratings;

    for (auto move : moves)
        ratings.push_back(RateByPosition(move, state));

    return ratings;
}

std::vector<int> UTTTAI::RateMovesByNextBoardPosition(const std::vector<Move> &moves, const AssessedState &state)
{
    std::vector<int> ratings;

    for (auto move : moves) {
        std::array<Player, 9> nextSubBoard{Player::None};

        int macroMove = move.x / 3 + 3 * (move.y / 3);
        int nextMacroBoardIndex = move.x % 3 + 3 * (move.y % 3);
        if(macroMove == nextMacroBoardIndex)
            nextSubBoard = TTT::DoMove(state.state.subBoards[nextMacroBoardIndex], nextMacroBoardIndex, state.state.player);
        else
            nextSubBoard = state.state.subBoards[nextMacroBoardIndex];

        if(TTT::GetMoves(nextSubBoard).empty()) // Next player gets to choose which macro-field to play on.
            ratings.push_back(-20);                 // Allowing opponent to pick is a bad choice, and should ...
                                                    // ... only be done when there's no other option available.
        else
        {   // Preferably send the opponent to the least important macro-field, with the lowest winning chance.
            // A field that rates higher for defensive/offensive relevance will rate lower.
            int rating = 0;
            rating -= state.macroFieldWorthsDefensive[nextMacroBoardIndex];
            rating -= state.macroFieldWorthsOffensive[nextMacroBoardIndex];
            ratings.push_back(rating);
        }
    }

    // Fetch best rating moves
    std::vector<int> bestMoves = BestRatingIndicesOfList(ratings);

    // If there are still moves that rate equally, try ranking them
    if(bestMoves.size() > 1 && bestMoves[0] != -20)
    {
        // Moves making opponent play on a sub-board where he has a lower chance of winning rate higher
        for (int bestMove : bestMoves) {
            int nextMacroBoardIndex = moves[bestMove].x % 3 + 3 * (moves[bestMove].y % 3);

            ratings[bestMove] += state.minMovesToPartialLosses[nextMacroBoardIndex];
            ratings[bestMove] -= state.minMovesToPartialWins[nextMacroBoardIndex];
        }
    }

    return ratings;
}

AssessedState UTTTAI::AssessState(const State &state)
{
    AssessedState assessedState;
    assessedState.state = state;

    assessedState.maxMovesRemaining = 0;
    for(Board b : state.subBoards)
        assessedState.maxMovesRemaining += TTT::GetMoves(b).size();

    for(int b = 0; b < 9; b++) assessedState.minMovesToPartialWins[b] = TTT::GetMinimumMovesToWin(state.subBoards[b], state.player);
    for(int b = 0; b < 9; b++) assessedState.minMovesToPartialLosses[b] = TTT::GetMinimumMovesToWin(state.subBoards[b], state.opponent);

    // Find out whether or not each sub-board can be won by which player(s)
    for(int b = 0; b < state.subBoards.size(); b++) assessedState.potentialSubBoardWinners[b] = TTT::IsWinnableForPlayer(state.subBoards[b]);

    // Add indices of relevant macro-boards to list for every time it represents a win.
    // This will help us find out whether we want to send our opponent to such board.
    for (auto & win : TTT::wins) {
        Player a = assessedState.potentialSubBoardWinners[win[0]];
        Player b = assessedState.potentialSubBoardWinners[win[1]];
        Player c = assessedState.potentialSubBoardWinners[win[2]];

        if(a == Player::None || b == Player::None || c == Player::None) continue; // a, b or c is never None after this point
        if(a == b && a == c && a == Player::Both) {
            assessedState.potentialWinners = Player::Both;

            assessedState.relevantMacroIndicesOffensive.push_back(win[0]);
            assessedState.relevantMacroIndicesOffensive.push_back(win[1]);
            assessedState.relevantMacroIndicesOffensive.push_back(win[2]);

            assessedState.relevantMacroIndicesDefensive.push_back(win[0]);
            assessedState.relevantMacroIndicesDefensive.push_back(win[1]);
            assessedState.relevantMacroIndicesDefensive.push_back(win[2]);
        }
        else if((a == Player::X || a == Player::Both) && (b == Player::X || b == Player::Both) && (c == Player::X || c == Player::Both)) {
            if(state.player == Player::X) {
                assessedState.relevantMacroIndicesOffensive.push_back(win[0]);
                assessedState.relevantMacroIndicesOffensive.push_back(win[1]);
                assessedState.relevantMacroIndicesOffensive.push_back(win[2]);
            } else {
                assessedState.relevantMacroIndicesDefensive.push_back(win[0]);
                assessedState.relevantMacroIndicesDefensive.push_back(win[1]);
                assessedState.relevantMacroIndicesDefensive.push_back(win[2]);
            }

            if(assessedState.potentialWinners == Player::O) assessedState.potentialWinners = Player::Both;
            else if(assessedState.potentialWinners != Player::Both) assessedState.potentialWinners = Player::X;
        }
        else if((a == Player::O || a == Player::Both) && (b == Player::O || b == Player::Both) && (c == Player::O || c == Player::Both)) {
            if(state.player == Player::O) {
                assessedState.relevantMacroIndicesOffensive.push_back(win[0]);
                assessedState.relevantMacroIndicesOffensive.push_back(win[1]);
                assessedState.relevantMacroIndicesOffensive.push_back(win[2]);
            } else {
                assessedState.relevantMacroIndicesDefensive.push_back(win[0]);
                assessedState.relevantMacroIndicesDefensive.push_back(win[1]);
                assessedState.relevantMacroIndicesDefensive.push_back(win[2]);
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
        if(std::find(assessedState.relevantMacroIndicesOffensive.begin(), assessedState.relevantMacroIndicesOffensive.end(), TTT::wins[w][0]) != assessedState.relevantMacroIndicesOffensive.end()
           && std::find(assessedState.relevantMacroIndicesOffensive.begin(), assessedState.relevantMacroIndicesOffensive.end(), TTT::wins[w][1]) != assessedState.relevantMacroIndicesOffensive.end()
           && std::find(assessedState.relevantMacroIndicesOffensive.begin(), assessedState.relevantMacroIndicesOffensive.end(), TTT::wins[w][2]) != assessedState.relevantMacroIndicesOffensive.end())
            assessedState.minMovesToWin[w] = assessedState.minMovesToPartialWins[TTT::wins[w][0]] + assessedState.minMovesToPartialWins[TTT::wins[w][1]] + assessedState.minMovesToPartialWins[TTT::wins[w][2]];
        else
            assessedState.minMovesToWin[w] = 0;

        if(std::find(assessedState.relevantMacroIndicesDefensive.begin(), assessedState.relevantMacroIndicesDefensive.end(), TTT::wins[w][0]) != assessedState.relevantMacroIndicesDefensive.end()
           && std::find(assessedState.relevantMacroIndicesDefensive.begin(), assessedState.relevantMacroIndicesDefensive.end(), TTT::wins[w][1]) != assessedState.relevantMacroIndicesDefensive.end()
           && std::find(assessedState.relevantMacroIndicesDefensive.begin(), assessedState.relevantMacroIndicesDefensive.end(), TTT::wins[w][2]) != assessedState.relevantMacroIndicesDefensive.end())
            assessedState.minMovesToLoose[w] = assessedState.minMovesToPartialLosses[TTT::wins[w][0]] + assessedState.minMovesToPartialLosses[TTT::wins[w][1]] + assessedState.minMovesToPartialLosses[TTT::wins[w][2]];
        else
            assessedState.minMovesToLoose[w] = 0;
    }

    RiddlesIOLogger::Log(STATE_ASSESSMENT_DONE, {std::to_string(assessedState.maxMovesRemaining)});

    return assessedState;
}

std::vector<int> UTTTAI::RateMovesByMacroRelevance(const std::vector<Move> & moves, const AssessedState & assessedState)
{
    std::vector<int> ratings;
    for (auto move : moves) {
        int macroBoardIndex = move.x / 3 + 3 * (move.y / 3);
        ratings.push_back(assessedState.macroFieldWorthsOffensive[macroBoardIndex] + assessedState.macroFieldWorthsDefensive[macroBoardIndex]);
    }
    return ratings;
}

std::vector<int> BestRatingIndicesOfList(const std::vector<int> &vals)
{
    int bestVal = vals[0];
    std::vector<int> indicesOfHighestValues;

    for (int val : vals)
        if(val > bestVal) bestVal = val;

    for (int i = 0; i < vals.size(); i++)
        if(vals[i] == bestVal) indicesOfHighestValues.push_back(i);

    return indicesOfHighestValues;
}

template<class O>
std::vector<O> PickValuesAtIndicesOfList(const std::vector<O> &list, const std::vector<int> &indices)
{
    std::vector<O> data;
    for (int indice : indices)
        data.push_back(list[indice]);
    return data;
}

std::ostream & operator << (std::ostream &os, const std::vector<Move> &m)
{
    if(m.empty()) return os;

    os << "(X: " << m[0].x << " Y: " << m[0].y << ")";

    for(int mi = 1; mi < m.size(); mi++)
        os << ", (X: " << m[mi].x << " Y: " << m[mi].y << ")";

    return os;
}
