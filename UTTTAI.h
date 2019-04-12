#ifndef UTTTAI_H
#define UTTTAI_H

#include <functional>

#include "UTTT.h"
#include "TTT.h"

struct AssessedState {
    /// The state that has been assessed.
    State state;

    /// Can Player::X win? Can Player::O win or can Player::Both win?
    Player potentialWinners;

    /// Which players can win which sub-boards?
    std::array<Player, 9> potentialSubBoardWinners;

    /// How many moves should bot/opponent at least make to win corresponding sub-board?
    std::array<int, 9> minMovesToPartialWins;
    std::array<int, 9> minMovesToPartialLosses;

    /// Which sub-boards are relevant for bot/opponent to win current uttt-game?
    std::vector<int> relevantMacroIndicesOffensive;
    std::vector<int> relevantMacroIndicesDefensive;

    /// In how many potential wins of bot/opponent is each sub-board relevant?
    std::array<int, 9> macroFieldWorthOffensive;
    std::array<int, 9> macroFieldWorthDefensive;

    /// For each win, how many moves does bot/opponent have to make to reach it?
    std::array<int, 8> minMovesToWin;
    std::array<int, 8> minMovesToLoose;

    /// The max amount of moves that have to be played to fill up the rest of the entire board
    /// Just use the factorial function on this number to get the maximum amount of states
    /// you have to evaluate to find how a perfectly played match from this state will end.
    int maxMovesRemaining;
};

struct SelectionStage {
    std::string name;
    std::function<std::vector<int> (const std::vector<Move> &, const AssessedState &)> evaluate;
};

class UTTTAI {

public:
    /// Main move selection function.
    static Move FindBestMove(const State &state, const int &timeout);

    /// Gathers data from a game-state and pre-processes certain variables.
    static AssessedState AssessState(const State &state);


    /// Rates moves by short term consequences they might have, eliminates worst moves but very resource intensive
    static std::vector<int> RateMovesByMiniMaxAB(const std::vector<Move> & moves, const AssessedState & state);

    /// Rates moves by their feasibility in a regular game of Tic-Tac-Toe.
    static std::vector<int> RateMovesByTTTStrategies(const std::vector<Move> &moves, const AssessedState &state);

    /// Rates moves by the potential limits they might impose on the opponent.
    static std::vector<int> RateMovesByPosition(const std::vector<Move> & moves, const AssessedState & state);

    /// Rates moves by the moves it might allow the opponent to make in their following moves.
    static std::vector<int> RateMovesByNextBoardPosition(const std::vector<Move> & moves, const AssessedState & state);

    /// Rates moves by the difference they might make in the macro-game
    static std::vector<int> RateMovesByMacroRelevance(const std::vector<Move> & moves, const AssessedState & assessedState);

    /// Helper function used by MiniMaxAB algorithm.
    static int EvaluateState(const State & state);


    /// Helper function used by MiniMaxAB algorithm.
    static std::vector<State> GetChildStates(const State &state);

};

namespace RatingDefinitions
{
    enum class MiniMax {
        Win = 1,
        Loose = -1,
        Undecided = 0
    };

    enum class Position {
        Any = -1,
        Winnable_for_both = 0,
        Winnable_for_one = 1,
        Stalemate = 2
    };

    enum class TTTStrategies {
        Win = 100,
        Prevent_Loose = 50
    };
};


/// Operator overload to easily print lists of moves
std::ostream & operator << (std::ostream& os, const std::vector<Move> &m);

/// Helper function that returns the indices of the highest values in a vector
static std::vector<int> BestRatingIndicesOfList(const std::vector<int> &vals);

template<class O>
/// Helper function that composes a list of values picked from a list by their indices
static std::vector<O> PickValuesAtIndicesOfList(const std::vector<O> &list, const std::vector<int> &indices);

#endif