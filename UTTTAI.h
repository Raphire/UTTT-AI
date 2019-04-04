//
// Created by Jorn on 04/04/2019.
//

#ifndef UTTTPROBESTBOTEUW_UTTTAI_H
#define UTTTPROBESTBOTEUW_UTTTAI_H

#include "uttt.h"

#define INITIAL_SEARCH_DEPTH 1

class UTTTAI {
public:
    static int EvaluateState(const State & state, const Player & positive);
    static std::vector<State> GetChildStates(const State & state);

    static Move FindBestMove(const State &state, const int &timeout);
    static int RateByMacrogameFieldValue(Move &move, std::array<std::array<Player, 9>, 9> array);
    static int RateByPosition(const Move &move, const std::array<std::array<Player, 9>, 9> & b);

};

static std::vector<int> BestRatingIndicesOfList(const std::vector<int> &vals);
template<class O>
static std::vector<O> PickValuesAtIndicesOfList(const std::vector<O> &list, const std::vector<int> &indices);

static std::array<std::array<Player, 9>, 9> ParseSubBoards(const std::array<std::array<Player, 9>, 9> &state);


#endif //UTTTPROBESTBOTEUW_UTTTAI_H
