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

	std::vector<std::string> split(const std::string &s, char delim);
	void setting(std::string &key, std::string &value);
	void update(std::string &key, std::string &value);
	void move(int timeout);
	Move findBestMove(const State &state, const int &timeout);
	static int EvaluateState(const State & state, const Player & positive);
	static std::vector<State> GetChildStates(const State & state);

public:
	void run();
};

#endif // UTTTBOT_H

