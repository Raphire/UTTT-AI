// uttt.h
// Aswin van Woudenberg

#ifndef UTTT_H
#define UTTT_H

#include <array>
#include <vector>
#include <ctime>
#include <random>
#include <iterator>
#include <iostream>

struct Move { int x, y; };
enum class Player { None, X, O, Active, Both };

struct State {
	std::array<std::array<Player,9>,9> board;
	std::array<std::array<Player,3>,3> macroboard;

	State() {
		for (int r=0; r<9; r++)
			for (int c=0; c<9; c++)
				board[r][c] = Player::None;
		for (int r=0; r<3; r++)
			for (int c=0; c<3; c++)
				macroboard[r][c] = Player::Active;
	}
};

// used to get a random element from a container
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

std::ostream &operator<<(std::ostream& os, const Player &p);
std::ostream &operator<<(std::ostream& os, const State &s);
std::ostream &operator<<(std::ostream& os, const Move &m);

class uttt {
public:
    static Player getCurrentPlayer(const State &state);
    static State doMove(const State &state, const Move &m);
    static Player getWinner(const State &state);
    static Player getWinner(const State &state, int row, int col);
    static std::vector<Move> getMoves(const State &state);
    static std::array<Player, 9> GetSubBoard(std::array<std::array<Player, 9>, 9> macroBoard, int i, int i1);
    static std::array<std::array<Player, 9>, 9> GetSubBoards(std::array<std::array<Player, 9>, 9> macroBoard);
    static std::array<Player, 9> GetNextSubBoard(std::array<std::array<Player, 9>, 9> array, int i, int i1);
    static int GetMicroMove(const Move &m);

};
#endif // UTTT_H

