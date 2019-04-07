// utttai.cpp
// Jeffrey Drost

#include "utttai.h"
#include "TreeSearch.h"

Move UTTTAI::findBestMove(const State &state, const int &timeout, const int &timePerMove)
{
    auto turnStartTime = std::chrono::steady_clock::now();
    int timeElapsed;
    Move bestMove = Move{ -1, -1};
    Player me = getCurrentPlayer(state);
    std::vector<Move> moves = getMoves(state);
    const int moveSize = moves.size();

    // Edge cases...
    if (moves.empty()) std::cerr << "ERROR: Board appears to be full, yet AI is asked to pick a move!" << std::endl;
    if (moveSize == 1) return moves[0]; // Might occur later in matches

    std::vector<int> moveRatings;
    int searchDepth = INITIAL_SEARCH_DEPTH;

    do {
        if (searchDepth > INITIAL_SEARCH_DEPTH) std::cerr << "Enough time left to do another pass with depth: " << searchDepth << "." << std::endl;
        std::cerr << "Starting pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << " with a search depth of " << searchDepth << "." << std::endl;

        bool searchTreeExhausted = true;

        for (int i = 0; i < moves.size(); i++) {
            bool fullMoveTreeEvaluated = true;
            State child = doMove(state, moves[i]);
            moveRatings.push_back(TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, me, -1, +1, &fullMoveTreeEvaluated));
            if (moveRatings[i] == +1) {
                std::cerr << "Found a route to a guaranteed win... Breaking off search!" << std::endl;
                return moves[i];
            }
            if (!fullMoveTreeEvaluated) searchTreeExhausted = false;
            else std::cerr << "Exhausted search tree of move #" << i << "." << std::endl;
        }
        std::cerr << "Finished pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << "." << std::endl;
        if (searchTreeExhausted)
        {
            std::cerr << "Entire search tree was exhausted! Bot knows how this game will end if played perfectly by both sides." << std::endl;
            break;
        }
        else std::cerr << "MiniMax did not find definite outcome for a perfectly played match..." << std::endl;
        searchDepth++; // Increase search depth for next iteration.

        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    } while ( // Keep searching 1 level deeper if there's enough time left
            (timeElapsed * 3 < timePerMove && timeout > (5 * timePerMove))
            || // Game has a branching factor of 7, expect the time elapsed each iteration to be multiplied with this factor in worst case.
            (timeElapsed * 6 < timePerMove)
            );

    // Find the highest score amongst rated moves
    // There might be multiple moves with the same -best score, put all of them in a list
    std::vector<Move> bestMoves;
    int highestRating = moveRatings[0];
    for (int i = 0; i < moves.size(); i++) {
        if (moveRatings[i] > highestRating) {
            highestRating = moveRatings[i];
            bestMoves.clear();
            bestMoves.push_back(moves[i]);
        }
        else if (moveRatings[i] == highestRating)
            bestMoves.push_back(moves[i]);
    }

    if (highestRating == -1)
        std::cerr << "All examined moves result in a loss! Chances are I will lose." << std::endl;

    std::vector<Move> secondaryBestMoves;

    if(bestMoves.size() > 1) {
        secondaryBestMoves = findBestMicroMoves(state, bestMoves, me);

        if (secondaryBestMoves.size() > 1)
            bestMove = *select_randomly(secondaryBestMoves.begin(), secondaryBestMoves.end());
        else if (secondaryBestMoves.size() == 1)
            bestMove = secondaryBestMoves[0];
        else
            std::cerr << "ERROR: Secondary Best moves list is empty!" << std::endl;
    }
    else if (bestMoves.size() == 1)
        bestMove = bestMoves[0];
    else
        std::cerr << "ERROR: Best moves list is empty!" << std::endl;

    if (bestMove.x == -1 && bestMove.y == -1) {
        std::cerr << "ERROR: Best move not found!" << std::endl;
        bestMove = *select_randomly(bestMoves.begin(), bestMoves.end());
    }

    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Search yields optimal column to do move: #" << bestMove << std::endl;
    std::cerr << "Search for move finished in " << timeElapsed << " milliseconds." << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl << std::endl;

    return bestMove; // Return highest-rating move
}

std::vector<Move>  UTTTAI::findBestMicroMoves(const State &state, const std::vector<Move> &bestMoves, const Player &me){
    std::vector<Move> secondaryBestMoves;
    int highestMicroRating = -999;
    int microRating;
    auto turnStartTime = std::chrono::steady_clock::now();
    int timeElapsed;

    std::vector<MacroState> preferredBoards = GetPreferredMacroBoards(state, me);

    for(Move move : bestMoves){
        State child = doMove(state, move);
        std::cerr << "move: " << move << std::endl;
        microRating = EvaluateMicroState(GetMicroState(child, move, false), me);
        std::cerr << "score1: " << microRating << std::endl;
        microRating += EvaluateNextPossibilities(GetMicroState(child, move, true), me);
        std::cerr << "score2: " << microRating << std::endl;
        if(ttt::CloseWin(GetMicroState(state, move, false), me, true))
            microRating += 1;
        for(MacroState state : preferredBoards){
            if(move.x / 3 == state.x && move.y / 3 == state.y)
                microRating += 3;
            if(move.x % 3 == state.x && move.y % 3 == state.y)
                microRating -= 3;
        }
        std::cerr << "totalscore: " << microRating << std::endl;
        if(microRating > highestMicroRating){
            highestMicroRating = microRating;
            secondaryBestMoves.clear();
            secondaryBestMoves.push_back(move);
        }
        else if(microRating == highestMicroRating){
            secondaryBestMoves.push_back(move);
        }
    }

    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Evaluation yields: #" << secondaryBestMoves.size() << " different moves" << std::endl;
    std::cerr << "Search for micro move finished in " << timeElapsed << " milliseconds." << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl << std::endl;

    return secondaryBestMoves;
}

int UTTTAI::EvaluateState(const State &state, const Player &player)
{
    Player winner = getWinner(state);                           // Is there a winner?
    if (winner == player) return +5;						    // Bot has won in evaluated state
    if (winner == Player::None) return 0;						// No winner
    return -5;                                                  // Opponent has won in evaluated state
}

int UTTTAI::EvaluateMicroState(const MicroState &state, const Player &player)
{
    Player winner = ttt::GetWinner(state);                                          // Is there a winner?
    if (winner == player) return +5;						                        // Bot has won in evaluated state
    Player possibleWinner = ttt::IsWinnableBy(state);
    if (winner == Player::None) return 0;						                    // No winner
    if(possibleWinner != player && possibleWinner != Player::Both) return -3;       // This microstate can only be won by the enemy
    return -5;                                                                      // Opponent has won in evaluated state
}

int UTTTAI::EvaluateNextPossibilities(const MicroState &state, const Player &me){
    MicroState nextBoard = state;
    int score = 0;

    auto nextMoves = ttt::GetMoves(nextBoard);

    if(ttt::CloseWin(nextBoard, me, true)) score -= 2;      // Making this move would allow the opponent to block my win next microboard
    if(ttt::CloseWin(nextBoard, me, false)) score -= 2;     // Making this move would allow the opponent to win the next microboard
    if(nextMoves.size() == 0) score -= 4;                   // Making this move gives the opponent the most options, as he gets to choose from all microboards

    if(score != 0){
        return score;
    }

    Player nextWinnableBy = ttt::IsWinnableBy(nextBoard);

    // This board can still be won by both players, it is still of good use
    if(nextWinnableBy == Player::Both) return 0;

    // Someone can still win on this board, but sending the opponent here would be better than previous options
    if(nextWinnableBy == Player::X || nextWinnableBy == Player::O) return 1;

    // It would be ideal to force an opponent to move here, as this board is not of any use to anyone
    if(nextWinnableBy == Player::None) return 3;
}

std::vector<State> UTTTAI::GetChildStates(const State &state)
{
    std::vector<State> children;
    std::vector<Move> moves = getMoves(state);
    for (Move m : moves) children.push_back(doMove(state, m));
    return children;
}

MicroState UTTTAI::GetMicroState(const State &state, const Move &move, bool getNext){
    std::array<Player, 9> microState;
    int xStart;
    int yStart;

    if(!getNext) {
        xStart = (move.y / 3) * 3;
        yStart = (move.x / 3) * 3;
    }
    else {
        xStart = (move.y % 3) * 3;
        yStart = (move.x % 3) * 3;
    }

    int i = 0;

    for (int x = xStart; x < xStart + 3; x++) {
        for (int y = yStart; y < yStart + 3; y++) {
            microState[i] = state.board[x][y];
            i++;
        }
    }

    return microState;
}

std::vector<MacroState> UTTTAI::GetPreferredMacroBoards (const State &state, const Player &me){
    std::vector<MacroState> preferredBoards;

    // Horizontal
    if(state.macroboard[0][0] == state.macroboard[1][0] && state.macroboard[2][0] == Player::None && state.macroboard[0][0] == me) preferredBoards.push_back(MacroState{ 2, 0 });
    if(state.macroboard[0][0] == state.macroboard[2][0] && state.macroboard[1][0] == Player::None && state.macroboard[0][0] == me) preferredBoards.push_back(MacroState{ 1, 0 });
    if(state.macroboard[1][0] == state.macroboard[2][0] && state.macroboard[0][0] == Player::None && state.macroboard[1][0] == me) preferredBoards.push_back(MacroState{ 0, 0 });
    if(state.macroboard[0][1] == state.macroboard[1][1] && state.macroboard[2][1] == Player::None && state.macroboard[0][1] == me) preferredBoards.push_back(MacroState{ 2, 1 });
    if(state.macroboard[0][1] == state.macroboard[2][1] && state.macroboard[1][1] == Player::None && state.macroboard[0][1] == me) preferredBoards.push_back(MacroState{ 1, 1 });
    if(state.macroboard[1][1] == state.macroboard[2][1] && state.macroboard[0][1] == Player::None && state.macroboard[1][1] == me) preferredBoards.push_back(MacroState{ 0, 1 });
    if(state.macroboard[0][2] == state.macroboard[1][2] && state.macroboard[2][2] == Player::None && state.macroboard[0][2] == me) preferredBoards.push_back(MacroState{ 2, 2 });
    if(state.macroboard[0][2] == state.macroboard[2][2] && state.macroboard[1][2] == Player::None && state.macroboard[0][2] == me) preferredBoards.push_back(MacroState{ 1, 2 });
    if(state.macroboard[1][2] == state.macroboard[2][2] && state.macroboard[0][2] == Player::None && state.macroboard[1][2] == me) preferredBoards.push_back(MacroState{ 0, 2 });

    // Vertical
    if(state.macroboard[0][0] == state.macroboard[0][1] && state.macroboard[0][2] == Player::None && state.macroboard[0][0] == me) preferredBoards.push_back(MacroState{ 0, 2 });
    if(state.macroboard[0][0] == state.macroboard[0][2] && state.macroboard[0][1] == Player::None && state.macroboard[0][0] == me) preferredBoards.push_back(MacroState{ 0, 1 });
    if(state.macroboard[0][1] == state.macroboard[0][2] && state.macroboard[0][0] == Player::None && state.macroboard[0][1] == me) preferredBoards.push_back(MacroState{ 0, 0 });
    if(state.macroboard[1][0] == state.macroboard[1][1] && state.macroboard[1][2] == Player::None && state.macroboard[1][0] == me) preferredBoards.push_back(MacroState{ 1, 2 });
    if(state.macroboard[1][0] == state.macroboard[1][2] && state.macroboard[1][1] == Player::None && state.macroboard[1][0] == me) preferredBoards.push_back(MacroState{ 1, 1 });
    if(state.macroboard[1][1] == state.macroboard[1][2] && state.macroboard[1][0] == Player::None && state.macroboard[1][1] == me) preferredBoards.push_back(MacroState{ 1, 0 });
    if(state.macroboard[2][0] == state.macroboard[2][1] && state.macroboard[2][2] == Player::None && state.macroboard[2][0] == me) preferredBoards.push_back(MacroState{ 2, 2 });
    if(state.macroboard[2][0] == state.macroboard[2][2] && state.macroboard[2][1] == Player::None && state.macroboard[2][0] == me) preferredBoards.push_back(MacroState{ 2, 1 });
    if(state.macroboard[2][1] == state.macroboard[2][2] && state.macroboard[2][0] == Player::None && state.macroboard[2][1] == me) preferredBoards.push_back(MacroState{ 2, 0 });

    // Diagonal
    if(state.macroboard[0][0] == state.macroboard[1][1] && state.macroboard[2][2] == Player::None && state.macroboard[0][0] == me) preferredBoards.push_back(MacroState{ 2, 2 });
    if(state.macroboard[0][0] == state.macroboard[2][2] && state.macroboard[1][1] == Player::None && state.macroboard[0][0] == me) preferredBoards.push_back(MacroState{ 1, 1 });
    if(state.macroboard[1][1] == state.macroboard[2][2] && state.macroboard[0][0] == Player::None && state.macroboard[1][1] == me) preferredBoards.push_back(MacroState{ 0, 0 });
    if(state.macroboard[2][0] == state.macroboard[1][1] && state.macroboard[0][2] == Player::None && state.macroboard[2][0] == me) preferredBoards.push_back(MacroState{ 0, 2 });
    if(state.macroboard[2][0] == state.macroboard[0][2] && state.macroboard[1][1] == Player::None && state.macroboard[2][0] == me) preferredBoards.push_back(MacroState{ 1, 1 });
    if(state.macroboard[0][2] == state.macroboard[1][1] && state.macroboard[2][0] == Player::None && state.macroboard[2][1] == me) preferredBoards.push_back(MacroState{ 2, 0 });

    return preferredBoards;
}