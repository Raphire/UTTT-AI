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

enum class Player { None, X, O, Active };
struct Move { int x, y; };
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
Player getCurrentPlayer(const State &state);
State doMove(const State &state, const Move &m);
Player getWinner(const State &state);
std::vector<Move> getMoves(const State &state);

#endif // UTTT_H

