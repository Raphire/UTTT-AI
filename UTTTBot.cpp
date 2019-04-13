#include "UTTTBot.h"

#include <sstream>

#include "RiddlesIOLogger.h"
#include "UTTTAI.h"

void UTTTBot::run()
{
	std::string line;
	while (std::getline(std::cin, line))
		input(line);
}

void UTTTBot::input(std::string &line)
{
    inputLines.push_back(line);	// Cache input for later use

	std::vector<std::string> command = split(line, ' ');
	if (command[0] == "settings") {
		setting(command[1], command[2]);
	} else if (command[0] == "update" && command[1] == "game") {
		update(command[2], command[3]);
	} else if (command[0] == "action" && command[1] == "move") {
		move(std::stoi(command[2]));
	} else {
		RiddlesIOLogger::Log(ERROR_BOT_RECEIVED_UNKNOWN_INPUT, {line});
	}
}

void UTTTBot::move(int timeout)
{
	int turn = round*2-1;
    state.turn = state.player;
    this->timebank = timeout;
    if(state.player == Player::X) turn++;
	RiddlesIOLogger::Log(BEGIN_OF_SEARCH, {std::to_string(round), std::to_string(turn)});

	Move m = UTTTAI::FindBestMove(state, time_per_move);
	std::cout << "place_disc " << m << std::endl;

	std::vector<std::string> inputLastRound;

	// Copy bot-initialization input.
	inputLastRound.insert(inputLastRound.end(), &inputLines[0], &inputLines[5]);
	// Copy input given to bot to make it play last round.
	inputLastRound.insert(inputLastRound.end(), &inputLines[inputLines.size() - 4], &inputLines[inputLines.size()]);

	// Create line of code to create vector with input.
	std::stringstream ss;
	ss << "lines = {";

	// Input first line without comma.
	ss << "\"" << inputLastRound[0] << "\"";

	// Add each line to string separated by comma, surrounded by double quotes.
	for(int i = 1; i < inputLastRound.size(); i++) ss << ", \"" << inputLastRound[i] << "\"";

	// Conclude vector definition line.
	ss << "};";


	RiddlesIOLogger::Log(DEBUG_REPLAY_ROUND, {ss.str()});
}

void UTTTBot::update(std::string &key, std::string &value)
{
	if (key == "round") {
		round = std::stoi(value);
		state.round = round;
	} else if (key == "field") {
		std::vector<std::string> fields = split(value, ',');
		for (int r = 0; r < 9; r++) {
			for (int c = 0; c < 9; c++) {
				int macroBoard = c / 3 + 3 * (r / 3);
				int subMove = c % 3 + (r % 3) * 3;
				if (fields[r * 9 + c] == "0") state.subBoards[macroBoard][subMove] = Player::O;
				else if (fields[r * 9 + c] == "1") state.subBoards[macroBoard][subMove] = Player::X;
				else state.subBoards[macroBoard][subMove] = Player::None;

			}
		}
		for (int i = 0; i < 9; i++) {
			if (state.macroBoard[i] == Player::X || state.macroBoard[i] == Player::O)
				state.subGamesWinnableBy[i] = state.macroBoard[i];
			else
				state.subGamesWinnableBy[i] = TTTGame::IsWinnableForPlayer(state.subBoards[i]);
		}
		state.winsMe = 0; state.winsOpp = 0;
		for(auto win : TTTGame::wins) {
			if((state.subGamesWinnableBy[win[0]] == Player::X || state.subGamesWinnableBy[win[0]] == Player::Both)
			&& (state.subGamesWinnableBy[win[1]] == Player::X || state.subGamesWinnableBy[win[1]] == Player::Both)
			&& (state.subGamesWinnableBy[win[2]] == Player::X || state.subGamesWinnableBy[win[2]] == Player::Both))
				state.player == Player::X ? state.winsMe++ : state.winsOpp++;
			if((state.subGamesWinnableBy[win[0]] == Player::O || state.subGamesWinnableBy[win[0]] == Player::Both)
		    && (state.subGamesWinnableBy[win[1]] == Player::O || state.subGamesWinnableBy[win[1]] == Player::Both)
		    && (state.subGamesWinnableBy[win[2]] == Player::O || state.subGamesWinnableBy[win[2]] == Player::Both))
				state.player == Player::O ? state.winsMe++ : state.winsOpp++;
		}


	} else if (key == "macroboard") {
		std::vector<std::string> fields = split(value, ',');
		for (int i = 0; i < 9; i++) {
			if (fields[i] == "-1") {
				state.macroBoard[i] = Player::Active;
			} else if (fields[i] == "0") {
				state.macroBoard[i] = Player::O;
			} else if (fields[i] == "1") {
				state.macroBoard[i] = Player::X;
			} else {
				state.macroBoard[i] = Player::None;
			}
		}
	}
}

void UTTTBot::setting(std::string &key, std::string &value)
{
	if (key == "timebank") {
		timebank = std::stoi(value);
	} else if (key == "time_per_move") {
		time_per_move = std::stoi(value);
		state.time_per_move = time_per_move;
	} else if (key == "player_names") {
		std::vector<std::string> names = split(value, ',');
		player_names[0] = names[0];
		player_names[1] = names[1];
	} else if (key == "your_bot") {
		your_bot = value;
	} else if (key == "your_botid") {
		your_botid = std::stoi(value);
		state.player = your_botid == 0 ? Player::O : Player::X;
		state.opponent = state.player == Player::X ? Player::O : Player::X;
	}
}

std::vector<std::string> UTTTBot::split(const std::string &s, char delim)
{
	std::vector<std::string> elements;
	std::stringstream ss(s);
	std::string item;

	while (std::getline(ss, item, delim)) elements.push_back(item);

	return elements;
}
