// utttai.cpp
// Jeffrey Drost

#include "utttai.h"
#include "TreeSearch.h"

// Finds the best next move for the bot, using minimax alphabeta and various other rules to determine what moves are best.
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
            moveRatings.push_back(TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, me, -50, +50, &fullMoveTreeEvaluated));
            if (moveRatings[i] >= +1) {
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

    // Find the moves with the highest score
    // There might be multiple moves with the same score
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

    if (highestRating <= -1)
        std::cerr << "All examined moves result in a loss! Chances are I will lose." << std::endl;

    std::vector<Move> secondaryBestMoves;

    // Evaluate the highest scoring moves using various evaluation methods
    if(bestMoves.size() > 1) {
        secondaryBestMoves = EvaluateBestMoves(state, bestMoves, me);

        //If multiple moves come out with the same score, select one of them randomly
        if (secondaryBestMoves.size() > 1)
            bestMove = *select_randomly(secondaryBestMoves.begin(), secondaryBestMoves.end());
        else if (secondaryBestMoves.size() == 1)
            bestMove = secondaryBestMoves[0];
        else
            std::cerr << "ERROR: secondaryBestMoves list is empty!" << std::endl;
    }
    else if (bestMoves.size() == 1)
        bestMove = bestMoves[0];
    else
        std::cerr << "ERROR: Best moves list is empty!" << std::endl;

    if (bestMove.x == -1 && bestMove.y == -1) {
        std::cerr << "ERROR: No best move was found!" << std::endl;
        bestMove = *select_randomly(bestMoves.begin(), bestMoves.end());
    }

    timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Search yields optimal position to do move: #" << bestMove << std::endl;
    std::cerr << "Search for move finished in " << timeElapsed << " milliseconds." << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl << std::endl;

    return bestMove; // Return highest-rating move
}

// Evaluates all given moves based on various rules and rates them accordingly
std::vector<Move>  UTTTAI::EvaluateBestMoves(const State &state, const std::vector<Move> &bestMoves, const Player &me){
    std::vector<Move> secondaryBestMoves;
    Player other = me == Player::X ? Player::O : Player::X;
    auto turnStartTime = std::chrono::steady_clock::now();
    int timeElapsed;
    int highestMicroRating = -999;
    int microRating;

    std::vector<MacroState> myPreferredBoards = GetPreferredMacroBoards(state, me, 2);
    std::vector<MacroState> enemyPreferredBoards = GetPreferredMacroBoards(state, other, 2);
    std::vector<MacroState> myLessPreferredBoards = GetPreferredMacroBoards(state, me, 1);
    std::vector<MacroState> enemyLessPreferredBoards = GetPreferredMacroBoards(state, other, 1);

    // Evaluate & rates all moves in bestMoves
    for(Move move : bestMoves){
        State child = doMove(state, move);
        std::cerr << "move: " << move << std::endl;

        microRating = EvaluateMicroState(GetMicroState(child, move, false), me);
        std::cerr << "score1: " << microRating << std::endl;

        microRating += EvaluateNextPossibilities(GetMicroState(child, move, true), me);
        std::cerr << "score2: " << microRating << std::endl;

        //Check if this move setups up two in a row for my bot
        if(!ttt::CheckSetups(GetMicroState(state, move, false), me), ttt::CheckSetups(GetMicroState(child, move, false), me))
            microRating += 5;

        //Check if this move blocks an enemy setup of two in a row
        if(ttt::CheckSetups(GetMicroState(state, move, false), other) && !ttt::CheckSetups(
                GetMicroState(child, move, false), other))
            microRating += 3;

        std::cerr << "score3: " << microRating << std::endl;

        Player winnable = ttt::IsWinnableBy(GetMicroState(state, move, false));

        // Check if move lines up with atleast 2 macroboards won by me
        for(MacroState macroState : myPreferredBoards){
            //Check if move is within a preferred board and assign score accordingly
            if (move.x / 3 == macroState.x && move.y / 3 == macroState.y) {
                if (winnable == me || winnable == Player::Both) {
                    microRating += 6;
                }
                if (enemyPreferredBoards.size() == 0) {
                    microRating += 3;
                    if(!ttt::CheckSetups(GetMicroState(state, move, false), me) &&
                            ttt::CheckSetups(GetMicroState(child, move, false), me))
                        microRating += 3;
                }
            } else if (move.x % 3 == macroState.x && move.y % 3 == macroState.y) {
                {
                    if (winnable == me || winnable == Player::Both) {
                        microRating -= 6;
                    }
                    if (enemyPreferredBoards.size() == 0) {
                        microRating -= 3;
                    }
                }
            }
            std::cerr << "myscore4: " << microRating << std::endl;
        }

        // Check if move lines up with atleast 2 macroboards won by the enemy
        for(MacroState macroState : enemyPreferredBoards){
            //Check if move is within an enemy preferred board and assign score accordingly
            if (move.x / 3 == macroState.x && move.y / 3 == macroState.y) {
                if (winnable == other || winnable == Player::Both) {
                    microRating += 6;
                }
                if (myPreferredBoards.size() == 0) {
                    microRating += 3;
                    if(enemyPreferredBoards.size() == 1 && ttt::CheckSetups(GetMicroState(state, move, false), other) && !ttt::CheckSetups(
                            GetMicroState(child, move, false), other))
                        microRating += 10;
                }
            } else if (move.x % 3 == macroState.x && move.y % 3 == macroState.y) {
                if (winnable == other || winnable == Player::Both) {
                    microRating -= 6;
                }
                if (myPreferredBoards.size() == 0) {
                    microRating -= 3;
                }
            }
            std::cerr << "enemyscore4: " << microRating << std::endl;
        }

        // Check if move lines up with atleast 1 macroboard won by me
        if(myPreferredBoards.size() == 0) {
            for(MacroState macroState : myLessPreferredBoards){
                //Check if this move is in line with a macroboard that this bot already won and assign points accordingly
                if (move.x / 3 == macroState.x && move.y / 3 == macroState.y) {
                    if (winnable == me || winnable == Player::Both) {
                        microRating += 3;
                    }
                } else if (move.x % 3 == macroState.x && move.y % 3 == macroState.y) {
                    if (winnable == me || winnable == Player::Both) {
                        microRating -= 3;
                    }
                }
                std::cerr << "1myscore14: " << microRating << std::endl;
            }
        }

        // Check if move lines up with atleast 1 macroboard won by the enemy
        if(enemyPreferredBoards.size() == 0) {
            for(MacroState macroState : enemyLessPreferredBoards){
                //Check if this move is in line with a macroboard that the enemy already won and assign points accordingly
                if (move.x / 3 == macroState.x && move.y / 3 == macroState.y) {
                    if (winnable == other || winnable == Player::Both) {
                        microRating += 3;
                    }
                } else if (move.x % 3 == macroState.x && move.y % 3 == macroState.y) {
                    if (winnable == other || winnable == Player::Both) {
                        microRating -= 3;
                    }
                }
                std::cerr << "enemyscore14: " << microRating << std::endl;
            }
        }

        std::cerr << "totalscore: " << microRating << std::endl;

        //Check if move is higher than or equal to current highestscore, if
        //higher it will clear the list, if the same it will add this move to the list.
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
    std::cerr << "Secondary evaluation yields: #" << secondaryBestMoves.size() << " different moves" << std::endl;
    std::cerr << "Secondary evaluation finished in " << timeElapsed << " milliseconds." << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl << std::endl;

    return secondaryBestMoves;
}

// Evaluate the state to see if there's a winner.
int UTTTAI::EvaluateState(const State &state, const Player &player)
{
    Player winner = getWinner(state);                           // Is there a winner?
    if (winner == player) return +50;						    // Bot has won in evaluated state
    if (winner == Player::None) return 0;						// No winner
    return -50;                                                 // Opponent has won in evaluated state
}

// Evaluate the microboard (one of the 3x3 boards) and check if there's a winner and whether or not the bot can still win
int UTTTAI::EvaluateMicroState(const MicroState &state, const Player &player)
{
    Player winner = ttt::GetWinner(state);                                           // Is there a winner?
    if (winner == player) return +10;						                         // Bot has won in evaluated state
    Player possibleWinner = ttt::IsWinnableBy(state);
    if(possibleWinner != player && possibleWinner != Player::Both) return -10;       // This microstate can only be won by the enemy
    if (winner == Player::None) return 0;						                     // No winner
    return -10;                                                                      // Opponent has won in evaluated state
}

// Evaluate the next posibilities of a state and assigns a score accordingly
int UTTTAI::EvaluateNextPossibilities(const MicroState &state, const Player &me){
    MicroState nextBoard = state;
    Player other = me == Player::X ? Player::O : Player::X;
    int score = 0;

    auto nextMoves = ttt::GetMoves(nextBoard);

    if(ttt::CheckSetups(nextBoard, me)) score -= 2;            // Making this move would allow the opponent to block my win next microboard
    if(ttt::CheckSetups(nextBoard, other)) score -= 2;         // Making this move would allow the opponent to win the next microboard
    if(nextMoves.size() == 0) score -= 5;                   // Making this move gives the opponent the most options, as he gets to choose from all microboards

    if(score != 0){
        return score;
    }

    Player nextWinnableBy = ttt::IsWinnableBy(nextBoard);

    // This board can still be won by both players, it is still of good use to both players
    if(nextWinnableBy == Player::Both) return 0;

    // Someone can still win on this board, but sending the opponent here would be better than the other options
    if(nextWinnableBy == Player::X || nextWinnableBy == Player::O) return 1;

    // It would be ideal to force an opponent to move here, as this board is not of any use to anyone
    if(nextWinnableBy == Player::None) return 5;
}

// Get all possible childstates of a given state
std::vector<State> UTTTAI::GetChildStates(const State &state)
{
    std::vector<State> children;
    std::vector<Move> moves = getMoves(state);
    for (Move m : moves) children.push_back(doMove(state, m));
    return children;
}

// Get the microboard (3x3 board) of a given state and a given move, with option to return
// either the current or next microboard
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

// Check whether there are x number of macroboard wins in a row for given player in the given state
std::vector<MacroState> UTTTAI::GetPreferredMacroBoards (const State &state, const Player &player, const int num){
    std::vector<MacroState> preferredBoards;
    State current;
    std::array<std::array<MacroState, 3>, 8> wins =
            {{
                     {MacroState{0, 0}, MacroState{0, 1}, MacroState{0, 2}},
                     {MacroState{1, 0}, MacroState{1, 1}, MacroState{1, 2}},
                     {MacroState{2, 0}, MacroState{2, 1}, MacroState{2, 2}},
                     {MacroState{0, 0}, MacroState{1, 0}, MacroState{2, 0}},
                     {MacroState{0, 1}, MacroState{1, 1}, MacroState{2, 1}},
                     {MacroState{0, 2}, MacroState{1, 2}, MacroState{2, 2}},
                     {MacroState{0, 0}, MacroState{1, 1}, MacroState{2, 2}},
                     {MacroState{2, 0}, MacroState{1, 1}, MacroState{0, 2}}
             }};

    // Clean up & add all necessary info of state to current for usage later
    for (int r=0; r<3; r++) {
        for (int c = 0; c < 3; c++) {
            if(state.macroboard[r][c] == Player::Active)
                current.macroboard[r][c] = Player::None;
            if(state.macroboard[r][c] == Player::X)
                current.macroboard[r][c] = Player::X;
            if(state.macroboard[r][c] == Player::O)
                current.macroboard[r][c] = Player::O;
        }
    }

    for(std::array<MacroState, 3> win : wins) {
        std::vector<MacroState> temp;
        int count = 0;

        for (MacroState m : win) {
            if (current.macroboard[m.y][m.x] == player) count++;
            else if (current.macroboard[m.y][m.x] == Player::None) {
                temp.push_back(m);
            }
            else if (current.macroboard[m.y][m.x] != Player::None) {
                count = 0;
                break;
            }
        }

        if(count == num && temp.size() != 0){
            for(MacroState ms : temp){
                preferredBoards.push_back(ms);
            }
            temp.clear();
        }
    }

    return preferredBoards;
}