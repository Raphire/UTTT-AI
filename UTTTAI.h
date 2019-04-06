//
// Created by Jorn on 04/04/2019.
//

#ifndef UTTTPROBESTBOTEUW_UTTTAI_H
#define UTTTPROBESTBOTEUW_UTTTAI_H

#include <functional>

#include "uttt.h"
#include "ttt.h"

#define INITIAL_SEARCH_DEPTH 1

class UTTTAI {

    struct SelectionStage {
        std::string name;
        int timeOut;
        std::function<std::vector<int> (const std::vector<Move> &, const State &)> evaluate;
    };

public:
    static int EvaluateState(const State & state);
    static std::vector<State> GetChildStates(const State &state);

    static Move FindBestMove(const State &state, const int &timeout);
    static int RateByMacrogameFieldValue(const Move & move, const State & state);
    static int RateByPosition(const Move & move, const State & state);


    static std::vector<int> RateMovesWithMiniMaxAB(const std::vector<Move> & moves, const State & state);
    static std::vector<int> RateMovesWithTTTStrats(const std::vector<Move> & moves, const State & state);
    static std::vector<int> RateMovesWithPosition(const std::vector<Move> & moves, const State &state);
    static std::vector<int> RateMovesWithNextBoardPosition(const std::vector<Move> & moves, const State & state);

    static Board GetMacroBoardStripped(const State &state);
    static Board GetPotentialMacroBoard(const State &state);
    static Player GetPotentialWinners(const State & state);

};

static std::vector<int> BestRatingIndicesOfList(const std::vector<int> &vals);
template<class O>
static std::vector<O> PickValuesAtIndicesOfList(const std::vector<O> &list, const std::vector<int> &indices);


#endif //UTTTPROBESTBOTEUW_UTTTAI_H
