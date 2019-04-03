// utttbot.cpp
// Aswin van Woudenberg

#include "utttbot.h"
#include "TreeSearch.h"

#include <iostream>
#include <sstream>
#include <chrono>

#define INITIAL_SEARCH_DEPTH 1

void UTTTBot::run() {
	std::string line;
	while (std::getline(std::cin, line)) {
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
}

void UTTTBot::move(int timeout) {
    if(firstMove){
        firstMove = false;
        std::cout << "place_disc " << 4 << " " << 3 << std::endl;
    }else {
        Move m = findBestMove(state, timeout);
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

Move UTTTBot::findBestMove(const State &state, const int &timeout)
{
    auto turnStartTime = std::chrono::steady_clock::now();
    int timeElapsed;

	Move bestMove = Move{ -1,-1 };

	// Find all moves and rate them
	Player me = getCurrentPlayer(state);
	std::cerr << "player: " << me << std::endl;
	std::vector<Move> moves = getMoves(state);
	const int moveSize = moves.size();

	// Edge cases...
	if (moves.empty()) std::cerr << "ERROR: Board appears to be full, yet AI is asked to pick a move!" << std::endl;
	if (moveSize == 1) return moves[0]; // Might occur later in matches
	
	std::vector<int> moveRatings;
	int searchDepth = INITIAL_SEARCH_DEPTH;

	do {
		if (searchDepth > INITIAL_SEARCH_DEPTH) std::cerr << "Enough time left to do another pass with depth: " << searchDepth << "." << std::endl;
		std::cerr << "Starting pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << " with a search depth of " << searchDepth << "." << std::endl;

		bool searchTreeExhausted = true;

		std::cerr << moves.size() << std::endl;

		for (int i = 0; i < moves.size(); i++) {
			bool fullMoveTreeEvaluated = true;
			State child = doMove(state, moves[i]);
			moveRatings.push_back(TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, me, -1, +1, &fullMoveTreeEvaluated));
			if (moveRatings[i] == +1) {
				std::cerr << "Found a route to a guaranteed win... Breaking off search!" << std::endl;
				return moves[i];
			}
			if (!fullMoveTreeEvaluated) searchTreeExhausted = false;
			else std::cerr << "Exhausted search tree of move #" << i << "." << std::endl;
		}
		std::cerr << "Finished pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << "." << std::endl;
		//std::cerr << "Time elapsed: " << match.timeElapsedThisTurn() << "/" << match.time_per_move << " ms." << std::endl;
		if (searchTreeExhausted)
		{
			std::cerr << "Entire search tree was exhausted! Bot knows how this game will end if played perfectly by both sides." << std::endl;
			//break;
		}
		else std::cerr << "MiniMax did not find definite outcome for a perfectly played match..." << std::endl;
		searchDepth++; // Increase search depth for next iteration.

		timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
	} while ( // Keep searching 1 level deeper if there's enough time left, do not risk loosing time-bank time during first 2 rounds, its not worth it
		(timeElapsed * 3 < time_per_move && timeout > (5 * time_per_move) && round > 2)
		|| // Game has a branching factor of 7, expect the time elapsed each iteration to be multiplied with this factor in worst case.
		(timeElapsed * 6 < time_per_move)
		);

	// Find the highest score amongst rated moves
	int highestRating = moveRatings[0];
	for (int i = 0; i < moves.size(); i++)
		if (moveRatings[i] > highestRating) highestRating = moveRatings[i];

	if (highestRating == -1)
		std::cerr << "All examined moves result in a loss! Chances are i will lose." << std::endl;

	// There might be multiple moves with the same -best score, put all of them in a list
	std::vector<Move> bestMoves;
	for (int i = 0; i < moves.size(); i++)
		if (moveRatings[i] == highestRating)
			bestMoves.push_back(moves[i]);

	if (bestMoves.empty()) std::cerr << "ERROR: Best moves list is empty!" << std::endl;

    bestMove = *select_randomly(bestMoves.begin(), bestMoves.end());

	if (bestMove.x == -1 && bestMove.y == -1) std::cerr << "ERROR: Best move not found!" << std::endl;

    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Search yields optimal column to do move: #" << bestMove << std::endl;
    std::cerr << "Search for move finished in " << timeElapsed << " milliseconds." << std::endl;
    std::cerr << "Awaiting next turn..." << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl << std::endl;

	return bestMove; // Return highest-rating move
}

int UTTTBot::EvaluateState(const State & state, const Player & positive)
{
	Player winner = getWinner(state);                           // Is there a winner?
	if (winner == positive) return +1;						// Bot has won in evaluated state
	if (winner == Player::None) return 0;						// No winner, rate state with heuristics
	return -1;                                               // Opponent has won in evaluated state
}

std::vector<State> UTTTBot::GetChildStates(const State &state)
{
	std::vector<State> children;
	std::vector<Move> moves = getMoves(state);
	for (Move m : moves) children.push_back(doMove(state, m));
	return children;
}