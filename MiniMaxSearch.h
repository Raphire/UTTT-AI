//
// Created by Jorn on 09/04/2019.
//

#ifndef UTTTPROBESTBOTEUW_MINIMAXSEARCH_H
#define UTTTPROBESTBOTEUW_MINIMAXSEARCH_H

#include <vector>
#include <iostream>
#include <future>

template <class Node>
class MiniMaxSearch {

    class NodeEvaluator {
    public:
        NodeEvaluator(Node node, MiniMaxSearch * search)
        {
            mms = search;
            this->node = node;
            isEvaluated = false;
        }

        int dig(bool maximize)
        {
            if(isEvaluated) {
                if(children.size() == 0) return mms->evaluateBranch(node);
            }
        }
        Node node;
    private:
        MiniMaxSearch * mms;
        bool isEvaluated;
        int val;
        std::vector<NodeEvaluator> children;
    };

public:
    MiniMaxSearch(int (*evaluate)(const Node &), std::vector<Node> (*findChildNodes)(const Node &));
    void setMaxEvaluationTime(int milliseconds);
    std::vector<int> evaluateBranch(Node branch);
    std::vector<int> evaluateBranchConcurrent(Node branch);
private:
    int maxDepth;
    int timeInMs;
    Node rootNode;
    int nodesEvaluated, nodesTraversed;
    int (*evaluateNode)(const Node & node);
    std::vector<Node> (*findChildNodes)(const Node & node);

    int MiniMaxAB(Node branch, int (*evaluate)(const Node &), std::vector<Node> (*findChildNodes)(const Node &), int depth, bool maximize, int worstVal, int bestVal, bool * abortSearch);
    int MiniMaxBF(Node branch, int (*evaluate)(const Node &), std::vector<Node> (*findChildNodes)(const Node &), int depth, bool maximize, int worstVal, int bestVal, bool * abortSearch);

};

template<class Node>
MiniMaxSearch<Node>::MiniMaxSearch(int (*evaluate)(const Node &), std::vector<Node> (*findChildNodes)(const Node &))
{
    this->evaluateNode = evaluate;
    this->findChildNodes = findChildNodes;
}

template<class Node>
void MiniMaxSearch<Node>::setMaxEvaluationTime(int milliseconds)
{
    timeInMs = milliseconds;
}

template<class Node>
std::vector<int> MiniMaxSearch<Node>::evaluateBranch(Node branch)
{
    nodesEvaluated = 0;
    nodesTraversed = 0;
    std::vector<Node> children = findChildNodes(branch);
    std::vector<int> ratings;

    bool abort = false;

    for(Node n:children)
    {
        int score = MiniMaxSearch::MiniMaxAB(n, evaluateNode, findChildNodes, 0, false, -1, 1, &abort);
        if(score == 1) std::cerr << "MiniMax found a route to a guaranteed win!" << std::endl;
        ratings.push_back(score);
    }

    std::cerr << "MiniMax traversed " << nodesTraversed << " nodes and evaluated " << nodesEvaluated << " end-nodes." << std::endl;

    return ratings;
}

template<class Node>
std::vector<int> MiniMaxSearch<Node>::evaluateBranchConcurrent(Node branch)
{
    auto startTime = std::chrono::steady_clock::now();
    long long int timeElapsed;

    nodesEvaluated = 0;
    nodesTraversed = 0;
    std::vector<Node> children = findChildNodes(branch);
    std::vector<int> ratings;

    bool abort = false;
    int threadsRunning = children.size();

    std::vector<std::promise<int> *> promises;

    for(int i = 0; i < children.size(); i++)
    {
        Node child = children[i];
        std::promise<int> * promise = new std::promise<int>();

        std::thread(
            [promise, this, child, &abort, &threadsRunning] {
                int score = MiniMaxSearch::MiniMaxAB(child, evaluateNode, findChildNodes, 0, false, -1, 1, &abort);
                threadsRunning--;
                promise->set_value_at_thread_exit(score);
            }
        ).detach();

        promises.push_back(promise);
    }

    do {
        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
    }
    while (timeElapsed < 100 && threadsRunning != 0);

    if(threadsRunning == 0) std::cerr << "MiniMax traversed entire game tree within time-frame!" << std::endl;
    else
    {
        std::cerr << "Aborting " << threadsRunning << " search threads..." << std::endl;
        abort = true; // Notify threads to abort search gracefully...
        std::cerr << "Search did not have enough time to evaluate full game tree from passed node." << std::endl;
    }

    for(int i = 0; i < promises.size(); i++)
    {
        std::promise<int> * promise = promises[i];
        std::future<int> future = promise->get_future();
        future.wait();

        ratings.push_back(future.get());
        delete promise;
    }

    std::cerr << "MiniMax traversed " << nodesTraversed << " nodes and evaluated " << nodesEvaluated << " end-nodes." << std::endl;

    return ratings;
}


template<class Node>
int MiniMaxSearch<Node>::MiniMaxAB(Node branch, int (*evaluate)(const Node &), std::vector<Node> (*findChildNodes)(const Node &), int depth, bool maximize, int worstVal, int bestVal, bool * abortSearch)
{
    if(* abortSearch) return evaluate(branch);

    nodesTraversed++;
    // Get all child nodes with function passed as argument
    auto children = findChildNodes(branch);

    // This branch has no children, all we can do is evaluate it now
    if(children.empty()) {
        nodesEvaluated++;
        return evaluate(branch);
    }

    int value;
    if(maximize) {
        value = worstVal;
        for(Node child:children) {
            int childVal = MiniMaxAB(child, evaluate, findChildNodes, depth+1, false, worstVal, bestVal, abortSearch);
            if(childVal > value) value = childVal;
            if(value > worstVal) worstVal = value;
            if(worstVal >= bestVal) break;
        }
    } else {
        value = bestVal;
        for(Node child:children) {
            int childVal = MiniMaxAB(child, evaluate, findChildNodes, depth+1, true, worstVal, bestVal, abortSearch);
            if(childVal < value) value = childVal;
            if(value < bestVal) bestVal = value;
            if(worstVal >= bestVal) break;
        }
    }

    return value;
}

template<class Node>
int MiniMaxSearch<Node>::MiniMaxBF(Node branch, int (*evaluate)(const Node &), std::vector<Node> (*findChildNodes)(const Node &), int depth, bool maximize, int worstVal, int bestVal, bool *abortSearch)
{
    if(* abortSearch) return evaluate(branch);

    nodesTraversed++;
    // Get all child nodes with function passed as argument
    auto children = findChildNodes(branch);

    // This branch has no children, all we can do is evaluate it now
    if(children.empty()) {
        nodesEvaluated++;
        return evaluate(branch);
    }

    int value;
    if(maximize) {
        value = worstVal;
        for(Node child:children) {
            int childVal = MiniMaxAB(child, evaluate, findChildNodes, depth+1, false, worstVal, bestVal, abortSearch);
            if(childVal > value) value = childVal;
            if(value > worstVal) worstVal = value;
            if(worstVal >= bestVal) break;
        }
    } else {
        value = bestVal;
        for(Node child:children) {
            int childVal = MiniMaxAB(child, evaluate, findChildNodes, depth+1, true, worstVal, bestVal, abortSearch);
            if(childVal < value) value = childVal;
            if(value < bestVal) bestVal = value;
            if(worstVal >= bestVal) break;
        }
    }

    return value;
}


#endif //UTTTPROBESTBOTEUW_MINIMAXSEARCH_H
