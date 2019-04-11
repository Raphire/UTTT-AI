//
// Created by Jorn on 04/04/2019.
//

#ifndef UTTTPROBESTBOTEUW_UTTTAI_H
#define UTTTPROBESTBOTEUW_UTTTAI_H

#include <functional>

#include "UTTT.h"
#include "TTT.h"

#define INITIAL_SEARCH_DEPTH 2

/// Play-style of bot, AI might switch strategies in between rounds depending on each State's Assessment
/// Aggressive play-style is preferred when bot is ahead to snowball into a win, when bot appears to fall behind
/// it might switch into Defensive play-style to prevent a loss. When bot is first to move it should try to rush a
/// win against its opponent while it is ahead, as starter's advantages might decay as the match progresses.
// TODO: Implement various play-style's
enum class PlayStyle {
    Rush,
    Aggressive,
    Defensive,
    Balanced
};

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
    std::array<int, 9> macroFieldWorthsOffensive;
    std::array<int, 9> macroFieldWorthsDefensive;

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
    int timeOut;
    std::function<std::vector<int> (const std::vector<Move> &, const AssessedState &)> evaluate;
};

class UTTTAI {

public:
    /// Main move selection function
    static Move FindBestMove(const State &state, const int &timeout);

    /// Eliminator/Selector functions that rate given moves
    static std::vector<int> RateMovesByMiniMaxAB(const std::vector<Move> &moves, const AssessedState &state);
    static std::vector<int> RateMovesByTTTStrats(const std::vector<Move> &moves, const AssessedState &state);
    static std::vector<int> RateMovesByPosition(const std::vector<Move> &moves, const AssessedState &state);
    static std::vector<int> RateMovesByNextBoardPosition(const std::vector<Move> &moves, const AssessedState &state);
    static std::vector<int> RateMovesByMacroRelevance(const std::vector<Move> & moves, const AssessedState & assessedState);

    /// Helper functions
    static int EvaluateState(const State & state);
    static std::vector<State> GetChildStates(const State &state);
    static int RateByPosition(const Move & move, const AssessedState & assessedState);
    static AssessedState AssessState(const State &state);

};

std::ostream &operator<<(std::ostream& os, const std::vector<Move> &m);

static std::vector<int> BestRatingIndicesOfList(const std::vector<int> &vals);
template<class O>
static std::vector<O> PickValuesAtIndicesOfList(const std::vector<O> &list, const std::vector<int> &indices);


#endif //UTTTPROBESTBOTEUW_UTTTAI_H
