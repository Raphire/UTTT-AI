//
// Created by Jorn on 10/04/2019.
//

#ifndef UTTTPROBESTBOTEUW_MINIMAXSEARCH_H
#define UTTTPROBESTBOTEUW_MINIMAXSEARCH_H

#include <vector>

template <class Node>
class MiniMaxSearch {
public:
    /// Constructs search object for template class Node
    MiniMaxSearch(int (*evaluateEndNode)(const Node &), std::vector<Node> (*findChildNodes)(const Node &));

    /// Evaluates a branch until maxDepth has been reached, returns according to evaluateEndNode function
    std::vector<int> evaluateBranch(const Node & node, int maxDepth);

    /// Returns whether the last search evaluated the entire state tree with given depth
    bool getLastSearchFullyEvaluated();

    /// Returns the amount of nodes (game states) that were traversed during the last search
    int getLastSearchNumNodesTraversed();
private:
    /// Returns Object O's value of type V according to MiniMax algorithm with alpha-beta pruning.
    /// This function should be applicable to any 2 player zero-sum game.
    /// https://en.wikipedia.org/wiki/Minimax#Minimax_algorithm_with_alternate_moves
    /// https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning
    /// Function arguments alpha and beta should be the worst and best value possible of type V, respectively.
    int MiniMaxAB(Node branch, int depth, bool maximize, int worstVal, int bestVal);

    int (*evaluateNode)(const Node &);
    std::vector<Node> (*findChildNodes)(const Node &);
    int nodesTraversed = 0;
    bool fullSearchDone;
};

template<class Node>
MiniMaxSearch<Node>::MiniMaxSearch(int (*evaluateEndNode)(const Node &), std::vector<Node> (*findChildNodes)(const Node &))
{
    this->evaluateNode = evaluateEndNode;
    this->findChildNodes = findChildNodes;
}

template<class Node>
std::vector<int> MiniMaxSearch<Node>::evaluateBranch(const Node &node, int maxDepth)
{
    std::vector<int> scores;
    fullSearchDone = true;
    nodesTraversed = 0;

    std::vector<Node> children = findChildNodes(node);

    for(Node child : children)
        scores.push_back(MiniMaxAB(child, maxDepth, false, -1, 1));

    return scores;
}

/// See: https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
/// - Node <branch>: The state to investigate, all its child-states will be examined with a depth of <depth>
/// - int Evaluate(Node n, Player positive): This function should evaluate a node and return its score
/// - std::vector<Node> FindChildNodes(Node n): This function finds all valid next child states, explains game-logic to function
/// - bool maximize: Whether or not to maximize the player who is on move in root node 'branch'
/// - int worstVal: the worst score possible; usually gained when losing the game (used for recursion, int min recommended)
/// - int bestVal: the best score possible; usually gained when winning the game (used for recursion, int max recommended)
template<class Node>
int MiniMaxSearch<Node>::MiniMaxAB(Node branch, int depth, bool maximize, int worstVal, int bestVal)
{
    // Keep track of the amount of traversed nodes to help estimate future performance
    nodesTraversed++;

    // Get all child nodes with function passed as argument
    auto children = findChildNodes(branch);

    // This branch has no children, all we can do is evaluate it now
    if(children.empty()) {
        return evaluateNode(branch);
    }

    // Depth limit has been reached, return value of current node
    if(depth == 0) {
        fullSearchDone = false;
        return evaluateNode(branch);
    }

    int value;
    if(maximize) {
        value = worstVal;
        for(Node child:children) {
            int childVal = MiniMaxAB(child, depth-1, false, worstVal, bestVal);
            if(childVal > value) value = childVal;
            if(value > worstVal) worstVal = value;
            if(worstVal >= bestVal) break;
        }
    } else {
        value = bestVal;
        for(Node child:children) {
            int childVal = MiniMaxAB(child, depth-1, true, worstVal, bestVal);
            if(childVal < value) value = childVal;
            if(value < bestVal) bestVal = value;
            if(worstVal >= bestVal) break;
        }
    }

    return value;
}

template<class Node>
bool MiniMaxSearch<Node>::getLastSearchFullyEvaluated()
{
    return fullSearchDone;
}

template<class Node>
int MiniMaxSearch<Node>::getLastSearchNumNodesTraversed()
{
    return nodesTraversed;
}


#endif //UTTTPROBESTBOTEUW_MINIMAXSEARCH_H
