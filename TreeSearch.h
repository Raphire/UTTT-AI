// treesearch.h
// Jeffrey Drost

#ifndef TREESEARCH_H
#define TREESEARCH_H

#include <vector>

class TreeSearch {
public:
    template <class O>
    static int MiniMaxAB(O branch, int (*evaluate)(const O &, const Player &), std::vector<O> (*findChildNodes)(const O &), int depth, bool maximize, Player p, int worstVal, int bestVal, bool * isFullTreeEvaluated);

};

#endif //TREESEARCH_H


// treesearch.cpp
template<class O>
int TreeSearch::MiniMaxAB(O branch, int (*evaluate)(const O &, const Player &), std::vector<O> (*findChildNodes)(const O &), int depth, bool maximize, Player p, int worstVal, int bestVal, bool * isFullTreeEvaluated)
{
    // Get all child nodes with function passed as argument
    auto children = findChildNodes(branch);

    // This branch has no children, all we can do is evaluate it now
    if(children.empty()) {
        return evaluate(branch, p);
    }

    // Depth limit has been reached, return value of current node
    if(depth == 0) {
        *isFullTreeEvaluated = false;
        return evaluate(branch, p);
    }

    int value;
    if(maximize) {
        value = worstVal;
        for(O child:children) {
            int childVal = MiniMaxAB(child, evaluate, findChildNodes, depth-1, false, p, worstVal, bestVal, isFullTreeEvaluated);
            if(childVal > value) value = childVal;
            if(value > worstVal) worstVal = value;
            if(worstVal >= bestVal) break;
        }
    } else {
        value = bestVal;
        for(O child:children) {
            int childVal = MiniMaxAB(child, evaluate, findChildNodes, depth-1, true, p, worstVal, bestVal, isFullTreeEvaluated);
            if(childVal < value) value = childVal;
            if(value < bestVal) bestVal = value;
            if(worstVal >= bestVal) break;
        }
    }

    return value;
}

