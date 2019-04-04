// utttbot.h
// Aswin van Woudenberg

#ifndef UTTTBOT_H
#define UTTTBOT_H

#include <string>
#include <vector>
#include <chrono>
#include <iostream>

#include "uttt.h"

class UTTTBot {
	int timebank;
	int time_per_move;
    std::chrono::time_point<std::chrono::steady_clock> turnStartTime;
	std::string player_names[2];
	std::string your_bot;
	int your_botid;
	bool firstMove = true;

	int round;
	State state;

	std::vector<int> bestRatingIndicesOfList(std::vector<int> vals);
	std::vector<int> worstRatingIndicesOf(std::vector<int> inds);

	std::vector<std::vector<int>> wins = {
			{0, 1, 2}, {3, 4, 5}, {6, 7, 8},
			{0, 3, 6}, {1, 4, 7}, {2, 5, 8},
			{0, 4, 8}, {2, 4, 6}
	};

	template<class O>
	std::vector<O> pickValuesAtIndicesOfList(std::vector<O> list, std::vector<int> indices);
	std::vector<std::string> split(const std::string &s, char delim);
	void setting(std::string &key, std::string &value);
	void update(std::string &key, std::string &value);
	void move(int timeout);
	static int EvaluateState(const State & state, const Player & positive);
	static std::vector<State> GetChildStates(const State & state);

public:
	Move findBestMove(const State &state, const int &timeout);


	void run();

	static int RateByPosition(const Move &move, const std::array<std::array<Player, 9>, 9> & b);
	static int GetMicroMove(const Move &m);

	static std::array<Player, 9> GetMicroBoard(std::array<std::array<Player, 9>, 9> array, int i, int i1);
	static std::array<Player, 9> GetNextMicroBoard(std::array<std::array<Player, 9>, 9> array, int i, int i1);

	static int RateByMacrogameFieldValue(Move &move, std::array<std::array<Player, 9>, 9> array);
};

#endif // UTTTBOT_H

