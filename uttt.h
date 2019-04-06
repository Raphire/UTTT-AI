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

enum class Player {
    None = -2,
    X = 1,
    O = 0,
    Active = -1,
    Both = 2
};

struct State {
	std::array<std::array<Player,9>,9> board;
	std::array<Player,9> macroboard;

	std::array<std::array<Player,9>,9> subBoards;
	Player turn = Player::O;
	Player player, opponent;
	int round;
	Player winner = Player::None;

    int time_per_move;

    State() {
		for (int r=0; r<9; r++)
			for (int c=0; c<9; c++)
				board[r][c] = Player::None;
		for (int i = 0; i < 9; i++)
				macroboard[i] = Player::Active;
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
std::ostream &operator<<(std::ostream& os, const std::array<int, 9> &vals);

class uttt {
public:
    static State doMove(const State &state, const Move &m);
    static std::vector<Move> getMoves(const State &state);
    static int GetMicroMove(const Move &m);

};
#endif // UTTT_H

