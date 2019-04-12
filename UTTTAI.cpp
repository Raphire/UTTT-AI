#include "UTTTAI.h"

#include <chrono>

#include "RiddlesIOLogger.h"
#include "MiniMaxSearch.h"
#include "TTTAI.h"

Move UTTTAI::FindBestMove(const State &state, const int &timeout)
{
    auto turnStartTime = std::chrono::steady_clock::now();
    long long int timeElapsed;

    // Elimination stages used by bot from first to final.
    std::array<SelectionStage, 5> selectionStages =
            {
                    SelectionStage {
                        "MiniMaxAB tree search",
                        UTTTAI::RateMovesByMiniMaxAB
                    },
                    SelectionStage {
                        "Macro board selection",
                        UTTTAI::RateMovesByMacroRelevance
                    },
                    SelectionStage {
                        "TTT strategies",
                        UTTTAI::RateMovesByTTTStrategies
                    },
                    SelectionStage {
                        "Potential outcome(s) of next board",
                        UTTTAI::RateMovesByNextBoardWinnable
                    },
                    SelectionStage {
                        "Next board macro-game relevance",
                        UTTTAI::RateMovesByNextBoardMacroRelevance
                    }
            };

    std::vector<Move> moves = UTTTGame::getMoves(state);
    std::vector<int> moveRatings;
    std::vector<Move> bestMoves = moves;

    AssessedState assessedState = AssessState(state);

    // Edge cases.
    if (moves.empty()) RiddlesIOLogger::Log(ERROR_DO_MOVE_ON_FINISHED_GAME, {});
    if (moves.size() == 1) return moves[0];

    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    RiddlesIOLogger::Log(BEGIN_ELIMINATION_OF_MOVES, {std::to_string(timeElapsed)});

    // Keep going through selection-stages until a single move remains.
    for(int s = 0; s < selectionStages.size(); s++)
    {
        // Rate remaining moves with current selection stage.
        moveRatings = selectionStages[s].evaluate(bestMoves, assessedState);

        // Find the highest rating moves and keep them in a separate list.
        std::vector<Move> newBestMoves = PickValuesAtIndicesOfList(bestMoves, BestRatingIndicesOfList(moveRatings));

        // Do some logging.
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
        RiddlesIOLogger::Log(SELECTIONSTAGE_PASSED_SUMMARY,{std::to_string(s+1),selectionStages[s].name,std::to_string(bestMoves.size() - newBestMoves.size()),std::to_string(bestMoves.size()),std::to_string(timeElapsed),std::to_string(moveRatings[BestRatingIndicesOfList(moveRatings)[0]]),RiddlesIOLogger::MovesToString(newBestMoves)});

        // If a single best move has been found, abort search and return it.
        if(newBestMoves.size() == 1)
        {
            RiddlesIOLogger::Log(SINGLE_BEST_MOVE_FOUND, {RiddlesIOLogger::MovesToString({newBestMoves[0]})});
            return newBestMoves[0];
        }

        // Update the new best move for the next selection stage.
        bestMoves = newBestMoves;
    }

    // Do some logging.
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    RiddlesIOLogger::Log(MULTIPLE_BEST_MOVES_FOUND, {std::to_string(timeElapsed), RiddlesIOLogger::MovesToString(bestMoves)});

    // Return best move.
    return bestMoves[0];
}

std::vector<int> UTTTAI::RateMovesByMiniMaxAB(const std::vector<Move> & moves, const AssessedState & state)
{
    // No need to search for wins or losses very early in the game, time is precious.
    if(state.state.round < 9) return std::vector<int>(moves.size());

    auto startTime = std::chrono::steady_clock::now();
    long long int timeElapsed;

    // Create MiniMax search, explain game-logic to MiniMax.
    MiniMaxSearch<State> mms = MiniMaxSearch<State>(EvaluateState, GetChildStates);

    // Use MiniMaxSearch to rate moves.
    std::vector<int> ratings = mms.evaluateBranchUntilTimeout(state.state, state.state.time_per_move);

    // Measure elapsed time for logging.
    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();

    // Do some conditional logging.
    if(mms.getLastSearchFullyEvaluated())
        RiddlesIOLogger::Log(MINIMAX_SEARCH_FINISHED_ALL_EVALUATED, {std::to_string(mms.getLastSearchNumNodesTraversed()), std::to_string(((double) mms.getLastSearchNumNodesTraversed()) / timeElapsed)});
    else
        RiddlesIOLogger::Log(MINIMAX_SEARCH_FINISHED, {std::to_string(mms.getLastSearchNumNodesTraversed()), std::to_string(((double) mms.getLastSearchNumNodesTraversed()) / timeElapsed)});

    return ratings;
}

int UTTTAI::EvaluateState(const State & state)
{
    Player winner = state.winner; // Return scores defined in enum RatingDefinitions::MiniMax
    if (winner == Player::None) return static_cast<int>(RatingDefinitions::MiniMax::Undecided);
    if (winner == state.player) return static_cast<int>(RatingDefinitions::MiniMax::Win);
    return static_cast<int>(RatingDefinitions::MiniMax::Loose);
}

std::vector<State> UTTTAI::GetChildStates(const State &state)
{
    std::vector<State> children;
    std::vector<Move> moves = UTTTGame::getMoves(state);

    for (Move m : moves)
        children.push_back(UTTTGame::doMove(state, m));

    return children;
}

std::vector<int> UTTTAI::RateMovesByTTTStrategies(const std::vector<Move> &moves, const AssessedState &state)
{
    std::vector<int> ratings;

    for (auto move : moves)
        ratings.push_back(TTTAI::RateMove(state, move));

    return ratings;
}

// TODO: Revisit this function
std::vector<int> UTTTAI::RateMovesByNextBoardWinnable(const std::vector<Move> &moves, const AssessedState &assessedState)
{
    std::vector<int> ratings;

    for (auto move : moves)
    {
        int score = 0;

        std::array<Player, 9> nextBoard = assessedState.state.subBoards[(move.x % 3) + 3 * (move.y % 3)];

        auto nextMoves = TTTGame::GetMoves(nextBoard);
        Player nextWinnableBy = TTTGame::IsWinnableForPlayer(nextBoard);

        if(nextMoves.empty())
            score = static_cast<int>(RatingDefinitions::NextBoardWinnable::Any); // Making this move gives the opponent the most options, as he gets the choice which micro board to play on

        // This board can still be won by both players, it is still of good use
        else if(nextWinnableBy == Player::Both)
            score = static_cast<int>(RatingDefinitions::NextBoardWinnable::Winnable_for_both);

        // Someone can still win on this board, but sending the opponent here would be better than previous options
        else if(nextWinnableBy == Player::X || nextWinnableBy == Player::O)
            score = static_cast<int>(RatingDefinitions::NextBoardWinnable::Winnable_for_one);

        // It would be ideal to force an opponent to move here, as this board is not of any use to anyone
        else if(nextWinnableBy == Player::None)
            score = static_cast<int>(RatingDefinitions::NextBoardWinnable::Stalemate);

        ratings.push_back(score);
    }

    return ratings;
}

// TODO: Review if function behaves as expected
std::vector<int> UTTTAI::RateMovesByNextBoardMacroRelevance(const std::vector<Move> &moves, const AssessedState &state)
{
    std::vector<int> ratings;

    for (auto move : moves) {
        std::array<Player, 9> nextSubBoard{Player::None};

        int macroMove = move.x / 3 + 3 * (move.y / 3);
        int nextMacroBoardIndex = move.x % 3 + 3 * (move.y % 3);
        if(macroMove == nextMacroBoardIndex)
            nextSubBoard = TTTGame::DoMove(state.state.subBoards[nextMacroBoardIndex], nextMacroBoardIndex, state.state.player);
        else
            nextSubBoard = state.state.subBoards[nextMacroBoardIndex];

        if(TTTGame::GetMoves(nextSubBoard).empty()) // Next player gets to choose which macro-field to play on.
            ratings.push_back(-20);             // Allowing opponent to pick is a bad choice, and should ...
                                                // ... only be done when there's no other option available.
        else
        {   // Preferably send the opponent to the least important macro-field, with the lowest winning chance.
            // A field that rates higher for defensive/offensive relevance will rate lower.
            int rating = 0;
            rating -= state.macroFieldWorthDefensive[nextMacroBoardIndex];
            rating -= state.macroFieldWorthOffensive[nextMacroBoardIndex];
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

// TODO: Review if function behaves as expected
AssessedState UTTTAI::AssessState(const State &state)
{
    AssessedState as;
    as.state = state;

    as.maxMovesRemaining = 0;
    for(Board b : state.subBoards)
        as.maxMovesRemaining += TTTGame::GetMoves(b).size();

    for(int b = 0; b < 9; b++) as.minMovesToPartialWins[b] = TTTGame::GetMinimumMovesToWin(state.subBoards[b], state.player);
    for(int b = 0; b < 9; b++) as.minMovesToPartialLosses[b] = TTTGame::GetMinimumMovesToWin(state.subBoards[b], state.opponent);

    // Find out whether or not each sub-board can be won by which player(s)
    for(int b = 0; b < state.subBoards.size(); b++) as.potentialSubBoardWinners[b] = TTTGame::IsWinnableForPlayer(state.subBoards[b]);

    // Add indices of relevant macro-boards to list for every time it represents a win.
    // This will help us find out whether we want to send our opponent to such board.
    for (auto & win : TTTGame::wins) {
        Player a = as.potentialSubBoardWinners[win[0]];
        Player b = as.potentialSubBoardWinners[win[1]];
        Player c = as.potentialSubBoardWinners[win[2]];

        if(a == Player::None || b == Player::None || c == Player::None) continue; // a, b or c is never None after this point
        if(a == b && a == c && a == Player::Both) {
            as.potentialWinners = Player::Both;
            as.relevantMacroIndicesOffensive.insert(as.relevantMacroIndicesOffensive.end(), &win[0], &win[3]);
            as.relevantMacroIndicesDefensive.insert(as.relevantMacroIndicesDefensive.end(), &win[0], &win[3]);
        }
        else if((a == Player::X || a == Player::Both) && (b == Player::X || b == Player::Both) && (c == Player::X || c == Player::Both)) {
            if(state.player == Player::X) as.relevantMacroIndicesOffensive.insert(as.relevantMacroIndicesOffensive.end(), &win[0], &win[3]);
            else as.relevantMacroIndicesDefensive.insert(as.relevantMacroIndicesDefensive.end(), &win[0], &win[3]);

            if(as.potentialWinners == Player::O) as.potentialWinners = Player::Both;
            else if(as.potentialWinners != Player::Both) as.potentialWinners = Player::X;
        }
        else if((a == Player::O || a == Player::Both) && (b == Player::O || b == Player::Both) && (c == Player::O || c == Player::Both)) {
            if(state.player == Player::O) as.relevantMacroIndicesOffensive.insert(as.relevantMacroIndicesOffensive.end(), &win[0], &win[3]);
            else as.relevantMacroIndicesDefensive.insert(as.relevantMacroIndicesDefensive.end(), &win[0], &win[3]);

            if(as.potentialWinners == Player::X) as.potentialWinners = Player::Both;
            else if(as.potentialWinners != Player::Both) as.potentialWinners = Player::O;
        }
    }

    // Assess the offensive and defensive value of each macro-board, this can help us decide which
    // macro-board  we want to win first, or which one we do not want our opponent to win anytime soon...
    for(int i = 0; i < 9; i++) as.macroFieldWorthOffensive[i] = std::count(as.relevantMacroIndicesOffensive.begin(), as.relevantMacroIndicesOffensive.end(), i);
    for(int i = 0; i < 9; i++) as.macroFieldWorthDefensive[i] = std::count(as.relevantMacroIndicesDefensive.begin(), as.relevantMacroIndicesDefensive.end(), i);

    // Assess the minimum amount of moves each player has to make to reach their respective win conditions.
    for(int w = 0; w < 8; w++)
    {
        if(std::find(as.relevantMacroIndicesOffensive.begin(), as.relevantMacroIndicesOffensive.end(), TTTGame::wins[w][0]) != as.relevantMacroIndicesOffensive.end()
           && std::find(as.relevantMacroIndicesOffensive.begin(), as.relevantMacroIndicesOffensive.end(), TTTGame::wins[w][1]) != as.relevantMacroIndicesOffensive.end()
           && std::find(as.relevantMacroIndicesOffensive.begin(), as.relevantMacroIndicesOffensive.end(), TTTGame::wins[w][2]) != as.relevantMacroIndicesOffensive.end())
            as.minMovesToWin[w] = as.minMovesToPartialWins[TTTGame::wins[w][0]] + as.minMovesToPartialWins[TTTGame::wins[w][1]] + as.minMovesToPartialWins[TTTGame::wins[w][2]];
        else
            as.minMovesToWin[w] = 0;

        if(std::find(as.relevantMacroIndicesDefensive.begin(), as.relevantMacroIndicesDefensive.end(), TTTGame::wins[w][0]) != as.relevantMacroIndicesDefensive.end()
           && std::find(as.relevantMacroIndicesDefensive.begin(), as.relevantMacroIndicesDefensive.end(), TTTGame::wins[w][1]) != as.relevantMacroIndicesDefensive.end()
           && std::find(as.relevantMacroIndicesDefensive.begin(), as.relevantMacroIndicesDefensive.end(), TTTGame::wins[w][2]) != as.relevantMacroIndicesDefensive.end())
            as.minMovesToLoose[w] = as.minMovesToPartialLosses[TTTGame::wins[w][0]] + as.minMovesToPartialLosses[TTTGame::wins[w][1]] + as.minMovesToPartialLosses[TTTGame::wins[w][2]];
        else
            as.minMovesToLoose[w] = 0;
    }

    RiddlesIOLogger::Log(STATE_ASSESSMENT_DONE, {std::to_string(as.maxMovesRemaining)});

    return as;
}

// TODO: Review if function behaves as expected
std::vector<int> UTTTAI::RateMovesByMacroRelevance(const std::vector<Move> & moves, const AssessedState & assessedState)
{
    std::vector<int> ratings;
    for (auto move : moves)
    {
        int macroBoardIndex = move.x / 3 + 3 * (move.y / 3);

        // Use data in previously assessed state to rate move.
        ratings.push_back(assessedState.macroFieldWorthOffensive[macroBoardIndex] + assessedState.macroFieldWorthDefensive[macroBoardIndex]);
    }
    return ratings;
}

std::vector<int> BestRatingIndicesOfList(const std::vector<int> &vals)
{
    int bestVal = vals[0];
    std::vector<int> indicesOfHighestValues;

    // Find the best value in this list.
    for (int val : vals)
        if(val > bestVal)
            bestVal = val;

    // Find the indices of the list that contain this value.
    for (int i = 0; i < vals.size(); i++)
        if(vals[i] == bestVal)
            indicesOfHighestValues.push_back(i);

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
