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
    Both = 2,
    Neither
};

struct State {
	std::array<std::array<Player,9>,9> subBoards;
	std::array<Player,9> macroBoard;
	std::array<Player,9> subGamesWinnableBy = {{Player::Both}};
	int winsMe = 8;
	int winsOpp = 8;

	Player turn = Player::O;
	Player player, opponent;
	int round = 1;
	Player winner = Player::None;

    int time_per_move;

    State() {
		for (int i = 0; i < 9; i++) macroBoard[i] = Player::Active;
	}
};

std::ostream &operator<<(std::ostream& os, const Player &p);
std::ostream &operator<<(std::ostream& os, const State &s);
std::ostream &operator<<(std::ostream& os, const Move &m);
std::ostream &operator<<(std::ostream& os, const std::array<int, 9> &vals);

/// This class defines Ultimate Tic-Tac-Toe game-flow
class UTTTGame {
public:
    static State doMove(const State &state, const Move &m);
    static std::vector<Move> getMoves(const State &state);
};

#endif

