#ifndef MINIMAXSEARCH_H
#define MINIMAXSEARCH_H

#include <vector>
#include <algorithm>

template <class Node>
class MiniMaxSearch {
public:
    /// Constructs search object for template class Node
    MiniMaxSearch(int (*evaluateEndNode)(const Node &), std::vector<Node> (*findChildNodes)(const Node &));

    /// Evaluates a branch until maxDepth has been reached, returns according to evaluateEndNode function
    std::vector<int> evaluateBranch(const Node & node, int maxDepth);

    /// Evaluates a branch as deep as possible until time-out has occurred
    std::vector<int> evaluateBranchUntilTimeout(const Node & node, int timeOutInMs);

    /// Returns whether the last search evaluated the entire state tree with given depth
    bool getLastSearchFullyEvaluated();

    /// Returns the amount of nodes (game states) that were traversed during the last search
    int getLastSearchNumNodesTraversed();

    /// Returns depth the last search was aborted at
    int getLastSearchDepth();
private:

    /// Evaluates the outcome of a perfectly-played match by either player (when depth limit is infinite)
    /// This function should be applicable to about any 2 player - turn based - zero-sum game.
    /// https://en.wikipedia.org/wiki/Minimax#Minimax_algorithm_with_alternate_moves
    /// https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning
    /// - Node <branch>: The state to investigate, all its child-states will be examined with a depth of <depth>
    /// - bool maximize: Whether or not player on the move in passed node will try to maximize score (as defined by evaluation function)
    /// - int worstVal: the worst score possible; usually gained when losing the game
    /// - int bestVal: the best score possible; usually gained when winning the game
    int MiniMaxAB(Node branch, int depth, bool maximize, int worstVal, int bestVal);

    /// int Evaluate(Node n, Player positive): This function should evaluate a node and return its score
    int (*evaluateNode)(const Node &);

    /// std::vector<Node> FindChildNodes(Node n): This function finds all valid next child states, explains game-logic to function
    std::vector<Node> (*findChildNodes)(const Node &);

    /// Internally used to keep track of algorithm performance
    int nodesTraversed;
    int searchDepth = 2;

    /// Holds whether or not the last search done has been fully-completed (as depth-limit/timeout might abort alg prematurely
    bool fullSearchDone;
};

template<class Node>
MiniMaxSearch<Node>::MiniMaxSearch(int (*evaluateEndNode)(const Node &), std::vector<Node> (*findChildNodes)(const Node &))
{
    this->evaluateNode = evaluateEndNode;
    this->findChildNodes = findChildNodes;

    nodesTraversed = 0;
    fullSearchDone = false;
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

template<class Node>
std::vector<int> MiniMaxSearch<Node>::evaluateBranchUntilTimeout(const Node &node, int timeOutInMs)
{
    auto startTime = std::chrono::steady_clock::now();
    nodesTraversed = 0;

    std::vector<Node> children = findChildNodes(node);
    std::vector<int> scores = std::vector<int>(children.size());
    std::vector<bool> isChildFullyEvaluated = std::vector<bool>(children.size(), false);
    std::vector<int> searchDepths = std::vector<int>(children.size(), 0);

    int currentBranch = 0;
    searchDepth = 3;

    do {
        // There's no need to evaluate this branch again if we already know its outcome
        if(!isChildFullyEvaluated[currentBranch])
        {
            fullSearchDone = true; // Reset full search done tracker
            scores[currentBranch] = MiniMaxAB(children[currentBranch], searchDepth, false, static_cast<int>(RatingDefinitions::MiniMax::Loose), static_cast<int>(RatingDefinitions::MiniMax::Win)); // Evaluate current branch
            if(fullSearchDone || scores[currentBranch] != 0) {
                isChildFullyEvaluated[currentBranch] = true; // Keep track of fully evaluated branches to improve performance
                if(scores[currentBranch] == static_cast<int>(RatingDefinitions::MiniMax::Loose)) scores[currentBranch] += searchDepth;  // Rate closest win highest
                if(scores[currentBranch] == static_cast<int>(RatingDefinitions::MiniMax::Win)) scores[currentBranch] -= searchDepth;    // Rate closest loose lowest
            }
        }
        // Have all branches(moves) been evaluated for this depth?: Increase depth, reset branch pointer
        if(currentBranch == children.size() - 1)
        {
            searchDepth++;
            currentBranch = 0;
        }
        else currentBranch++;

    } while
    (       // Stop search when timeout occurs, or search is completely done.
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() < timeOutInMs
            &&
            std::find(isChildFullyEvaluated.begin(), isChildFullyEvaluated.end(), false) != isChildFullyEvaluated.end()
    );

    // Update fullSearchDone for users outside of this class,
    // next line evaluates whether any of the child branches haven't been fully evaluated
    fullSearchDone = std::find(isChildFullyEvaluated.begin(), isChildFullyEvaluated.end(), false) == isChildFullyEvaluated.end();

    return scores;
}

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

template<class Node>
int MiniMaxSearch<Node>::getLastSearchDepth()
{
    return searchDepth;
}

#endif
