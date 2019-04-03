
#ifndef C4TEST_TREESEARCH_H
#define C4TEST_TREESEARCH_H

#include <vector>

class TreeSearch {
public:
    template <class O>
    /// Returns Object O's value of type V according to MiniMax algorithm with alpha-beta pruning.
    /// This function should be applicable to any 2 player zero-sum game.
    /// https://en.wikipedia.org/wiki/Minimax#Minimax_algorithm_with_alternate_moves
    /// https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning
    /// Function arguments alpha and beta should be the worst and best value possible of type V, respectively.
    static int MiniMaxAB(O branch, int (*evaluate)(const O &, const Player &), std::vector<O> (*findChildNodes)(const O &), int depth, bool maximize, Player p, int worstVal, int bestVal, bool * isFullTreeEvaluated);

};

#endif //C4TEST_TREESEARCH_H


/// TreeSearch.tpp

/// It so appears functions using template arguments cannot be defined in a separate files
/// Therefor, search implementations are defined here
/// See: https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
/// Arguments: (O = 'Node')
/// - Node <branch>: The state to investigate, all its child-states will be examined with a depth of <depth>
/// - int Evaluate(Node n, Player positive): This function should evaluate a node and return its score
/// - std::vector<Node> FindChildNodes(Node n): This function finds all valid next child states, explains game-logic to function
/// - bool maximize: Whether or not to maximize the player who is on move in root node 'branch'
/// - int worstVal: the worst score possible; usually gained when losing the game (used for recursion, int min recommended)
/// - int bestVal: the best score possible; usually gained when winning the game (used for recursion, int max recommended)
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

