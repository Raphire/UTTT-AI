#ifndef UTTTBOT_H
#define UTTTBOT_H

#include <string>
#include <vector>
#include <chrono>
#include <iostream>

#include "UTTT.h"

/// This class handles communication between AI and the Riddles.io Ultimate Tic-Tac-Toe platform
class UTTTBot {
	int timebank;
	int time_per_move;
    std::chrono::time_point<std::chrono::steady_clock> turnStartTime;
	std::string player_names[2];
	std::string your_bot;
	int your_botid;
	int round;
	State state;

	std::vector<std::string> inputLines;

	std::vector<std::string> split(const std::string &s, char delim);
	void setting(std::string &key, std::string &value);
	void update(std::string &key, std::string &value);
	void move(int timeout);

public:
	void run();
    void input(std::string &line);

};

#endif

