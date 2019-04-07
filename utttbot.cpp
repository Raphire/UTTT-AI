// utttbot.cpp
// Jeffrey Drost

#include "utttbot.h"

#include <iostream>
#include <sstream>
#include <chrono>

void UTTTBot::run() {
	std::string line;
	while (std::getline(std::cin, line)) input(line);
}

void UTTTBot::move(int timeout) {
    if(firstMove){
        firstMove = false;

        Move r = Move{4,4};

        std::cout << "place_disc " << r << std::endl;
    }else {
        Move m = UTTTAI::findBestMove(state, timeout, time_per_move);
        std::cout << "place_disc " << m << std::endl;
    }
}

void UTTTBot::update(std::string &key, std::string &value) {
	if (key == "round") {
		round = std::stoi(value);
	} else if (key == "field") {
		int row = 0;
		int col = 0;
		std::vector<std::string> fields = split(value, ',');
		for (std::string &field : fields) {
			if (field == "0") {
				state.board[row][col] = Player::X; 
			} else if (field == "1") {
				state.board[row][col] = Player::O;
			} else {
				state.board[row][col] = Player::None;
			}
			col++;
			if (col == 9) {
				row++; 
				col = 0;
			}
		}
	} else if (key == "macroboard") {
		int row = 0;
		int col = 0;
		std::vector<std::string> fields = split(value, ',');
		for (std::string &field : fields) {
			if (field == "-1") {
				state.macroboard[row][col] = Player::Active;
			} else if (field == "0") {
				state.macroboard[row][col] = Player::X;
			} else if (field == "1") {
				state.macroboard[row][col] = Player::O;
			} else {
				state.macroboard[row][col] = Player::None;
			}
			col++;
			if (col == 3) {
				row++;
				col = 0;
			}
		}
	}
}

void UTTTBot::setting(std::string &key, std::string &value) {
	if (key == "timebank") {
		timebank = std::stoi(value);
	} else if (key == "time_per_move") {
		time_per_move = std::stoi(value);
	} else if (key == "player_names") {
		std::vector<std::string> names = split(value, ',');
		player_names[0] = names[0];
		player_names[1] = names[1];
	} else if (key == "your_bot") {
	    if(value == player_names[0]){
            firstMove = true;
	    }
		your_bot = value;
	} else if (key == "your_botid") {
		your_botid = std::stoi(value);
	}
}

std::vector<std::string> UTTTBot::split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

void UTTTBot::input(std::basic_string<char> & line)
{
    std::vector<std::string> command = split(line, ' ');
    if (command[0] == "settings") {
        setting(command[1], command[2]);
    } else if (command[0] == "update" && command[1] == "game") {
        update(command[2], command[3]);
    } else if (command[0] == "action" && command[1] == "move") {
        move(std::stoi(command[2]));
    } else {
        std::cerr << "Unknown command: " << line << std::endl;
    }
}
