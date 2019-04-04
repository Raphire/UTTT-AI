// utttbot.cpp
// Aswin van Woudenberg

#include "utttbot.h"
#include "TreeSearch.h"
#include "ttt.h"

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

void UTTTBot::move(int timeout)
{
    if(firstMove) {
        firstMove = false;
        if(your_botid == 0) {
			std::cout << "place_disc " << 4 << " " << 3 << std::endl; // Center win is best win
			return;
		}
    }
	Move m = findBestMove(state, timeout);
	std::cout << "place_disc " << m << std::endl;

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
	    firstMove = true;
		timebank = std::stoi(value);
	} else if (key == "time_per_move") {
		time_per_move = std::stoi(value);
	} else if (key == "player_names") {
		std::vector<std::string> names = split(value, ',');
		player_names[0] = names[0];
		player_names[1] = names[1];
	} else if (key == "your_bot") {
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
    long long int timeElapsed;

	// Find all moves and rate them
	Player me = getCurrentPlayer(state);
	std::vector<Move> moves = getMoves(state);

    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Starting move search for round: " << round << " as Player " <<  me << "." << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

	// Edge cases...
	if (moves.empty()) std::cerr << "ERROR: Board appears to be full, yet AI is asked to pick a move!" << std::endl;
	if (moves.size() == 1) return moves[0]; // Might occur later in matches
	int searchDepth = INITIAL_SEARCH_DEPTH;


	/// STAGE ONE
	timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
	std::vector<int> primaryRatings = std::vector<int>();
	std::cerr << "______________________________________________________________________________________________" << std::endl;
	std::cerr << "Stage #1 (Elimination): Evaluating a total of " << moves.size() << " moves using MinMaxAB. (" << timeElapsed << " ms)" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

    // TODO: This stage can probably be optimized a lot by rewriting the GetChildStates and EvaluateState functions, increasing search depth which will in its turn dramatically increase AI performance

    do {
		std::cerr << "Starting pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << ": Search depth of " << searchDepth << "." << std::endl;

		bool searchTreeExhausted = true;
		for (int i = 0; i < moves.size(); i++)
		{
			bool fullMoveTreeEvaluated = true;
			State child = doMove(state, moves[i]);
			primaryRatings.push_back(TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, me, -1, +1, &fullMoveTreeEvaluated));

			if (primaryRatings[i] == +1)
			{
				std::cerr << "Found a route to a guaranteed win... Breaking off search!" << std::endl;
				return moves[i];
			}

			if (!fullMoveTreeEvaluated) searchTreeExhausted = false;
			else std::cerr << "Exhausted search tree of move #" << i << "." << std::endl;
		}
		if (searchTreeExhausted)
		{
			std::cerr << "Entire search tree was exhausted! Bot knows how this game will end if played perfectly by both sides." << std::endl;
			break;
		}
		searchDepth++; // Increase search depth for next iteration.

		timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
	}
    while ( // Keep searching 1 level deeper if there's enough time left, do not risk loosing time-bank time during first 2 rounds, its not worth it
		(timeElapsed * 3 < time_per_move && timeout > (5 * time_per_move) && round > 2)
		|| // Game has a branching factor of 7, expect the time elapsed each iteration to be multiplied with this factor in worst case.
		(timeElapsed * 6 < time_per_move)
		);

	std::vector<Move> bestMoves = pickValuesAtIndicesOfList(moves, bestRatingIndicesOfList(primaryRatings));
    std::cerr << "Stage #1 eliminated " << moves.size() - bestMoves.size() << " of " << moves.size() << " moves." << std::endl;
	std::cerr << "Highest rating moves are rated " << primaryRatings[bestRatingIndicesOfList(primaryRatings)[0]] << "." << std::endl << std::endl;

	if(bestMoves.size() == 1) {
		timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
		std::cerr << "Found a single best move. Breaking of search! (" << timeElapsed << " ms) " << std::endl;
		return bestMoves[0];
	}


	/// STAGE TWO
	timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
	std::vector<int> secondaryRatings = std::vector<int>();
	std::cerr << "______________________________________________________________________________________________" << std::endl;
	std::cerr << "Stage #2 (Elimination): Evaluating a total of " << bestMoves.size() << " moves using MinMaxAB for sub-games. (" << timeElapsed << " ms)" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

    // TODO: This stage is scuffed AF, define better ways to find who's turn it is in a sub-game or remove this stage completely...

    bool b = false;
    for (int m = 0; m < bestMoves.size(); m++)
    {
        Board microBoard = GetMicroBoard(state.board, bestMoves[m].x / 3, bestMoves[m].y / 3);
        int microX = bestMoves[m].x % 3;
        int microY = bestMoves[m].y % 3;
        int pos = microX * microY + microY;
        Board child = ttt::DoMove(microBoard, pos, me);
        int score = TreeSearch::MiniMaxAB(child, ttt::EvaluateBoard, ttt::GetChildren, 8, true, me, -1, 1, &b);
        secondaryRatings.push_back(score);
    }
    std::vector<Move> bestMovesSecondary = pickValuesAtIndicesOfList(bestMoves,
																	 bestRatingIndicesOfList(secondaryRatings));
    std::cerr << "Stage #2 eliminated " << bestMoves.size() - bestMovesSecondary.size() << " of " << bestMoves.size() << " moves." << std::endl;
	std::cerr << "Highest rating moves are rated " << secondaryRatings[bestRatingIndicesOfList(secondaryRatings)[0]] << "." << std::endl << std::endl;


	if(bestMovesSecondary.size() == 1)
    {
		timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
		std::cerr << "Found a single best move. Breaking of search! (" << timeElapsed << " ms) " << std::endl;
        return bestMovesSecondary[0];
    }


    /// STAGE THREE
	timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
	std::vector<int> tertiaryRatings = std::vector<int>();
	std::cerr << "______________________________________________________________________________________________" << std::endl;
	std::cerr << "Stage #3 (Elimination): Evaluating a total of " << bestMovesSecondary.size() << " moves using position rating. (" << timeElapsed << " ms)" << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl;

    for(int i = 0; i < bestMovesSecondary.size(); i++)
        tertiaryRatings.push_back(RateByPosition(bestMovesSecondary[i], state.board));

    std::vector<Move> bestMovesTertiary = pickValuesAtIndicesOfList(bestMovesSecondary,
																	bestRatingIndicesOfList(tertiaryRatings));
    std::cerr << "Stage #3 eliminated " << bestMovesSecondary.size() - bestMovesTertiary.size() << " of " << bestMovesSecondary.size() << " moves." << std::endl;
    std::cerr << "Highest rating moves are rated " << tertiaryRatings[bestRatingIndicesOfList(tertiaryRatings)[0]] << "." << std::endl << std::endl;

    if(bestMovesTertiary.size() == 1) {
		timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
		std::cerr << "Found a single best move. Breaking of search! (" << timeElapsed << " ms) " << std::endl;
        return bestMovesTertiary[0];
    }


	/// STAGE FOUR
	timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
	std::vector<int> quaternaryRatings = std::vector<int>();
	std::cerr << "______________________________________________________________________________________________" << std::endl;
	std::cerr << "Stage #4 (Elimination): Evaluating a total of " << bestMovesTertiary.size() << " moves using position rating. (" << timeElapsed << " ms)" << std::endl;
	std::cerr << "______________________________________________________________________________________________" << std::endl;

	for(int i = 0; i < bestMovesTertiary.size(); i++)
		quaternaryRatings.push_back(RateByMacrogameFieldValue(bestMovesTertiary[i], state.board));

	std::vector<Move> bestMovesQuaternary = pickValuesAtIndicesOfList(bestMovesTertiary,
																	  bestRatingIndicesOfList(quaternaryRatings));
	std::cerr << "Stage #4 eliminated " << bestMovesTertiary.size() - bestMovesQuaternary.size() << " of " << bestMovesTertiary.size() << " moves." << std::endl;
	std::cerr << "Highest rating moves are rated " << quaternaryRatings[bestRatingIndicesOfList(quaternaryRatings)[0]] << "." << std::endl << std::endl;

	if(bestMovesQuaternary.size() == 1)
	{
		timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
		std::cerr << "Found a single best move. Breaking of search! (" << timeElapsed << " ms) " << std::endl;
		return bestMovesQuaternary[0];
	}

    /// STAGE RANDOM
	timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - turnStartTime).count();
	std::cerr << "Multiple optimal moves have been found. Selecting one randomly... (" << timeElapsed << " ms) " << std::endl;
    return *select_randomly(bestMovesQuaternary.begin(), bestMovesQuaternary.end());
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

std::vector<int> UTTTBot::bestRatingIndicesOfList(std::vector<int> vals)
{
	int bestVal = vals[0];
	std::vector<int> indicesOfHighestValues = std::vector<int>();

	for (int i = 0; i < vals.size(); i++)
		if(vals[i] > bestVal) bestVal = vals[i];

	for (int i = 0; i < vals.size(); i++)
		if(vals[i] == bestVal) indicesOfHighestValues.push_back(i);

	return indicesOfHighestValues;
}

std::vector<int> UTTTBot::worstRatingIndicesOf(std::vector<int> inds)
{
	int worstVal = inds[0];
	std::vector<int> worstInds = {0};
	for (int i = 0; i < inds.size(); i++) {
		if (inds[i] < worstVal) {
			worstVal = inds[i];
			worstInds = {i};
		} else if (inds[i] == worstVal) {
			worstInds.push_back(i);
		}
	}
	return worstInds;
}

template<class O>
std::vector<O> UTTTBot::pickValuesAtIndicesOfList(std::vector<O> list, std::vector<int> indices)
{
	std::vector<O> data = std::vector<O>();
	for(int i = 0; i < indices.size(); i++) data.push_back(list[indices[i]]);
	return data;
}

int UTTTBot::RateByPosition(const Move & move, const std::array<std::array<Player, 9>, 9> & b)
{
	int m = GetMicroMove(move);

	Board nextBoard = GetNextMicroBoard(b, move.x, move.y);

	auto nextMoves = ttt::GetMoves(nextBoard);
	if(nextMoves.size() == 0) return -1; // Making this move gives the opponent the most options, as he gets the choice which micro board to play on
	Player nextWinnableBy = ttt::IsWinnableBy(nextBoard);

	// This board can still be won by both players, it is still of good use
	if(nextWinnableBy == Player::Both) return 0;

	// Someone can still win on this board, but sending the opponent here would be better than previous options
	if(nextWinnableBy == Player::X || nextWinnableBy == Player::O) return 1;

	// It would be ideal to force an opponent to move here, as this board is not of any use to anyone
	if(nextWinnableBy == Player::None) return 2;
}

std::array<Player, 9> UTTTBot::GetMicroBoard(std::array<std::array<Player, 9>, 9> array, int i, int i1)
{
	Board b = std::array<Player, 9>();
	for(int x = 0; x < 3; x++) for(int y = 0; y < 3; y++) b[x*3 + y] = array[i*3 + x][i1*3 + y];
	return b;
}

int UTTTBot::GetMicroMove(const Move &m)
{
	int microX = m.x % 3;
	int microY = m.y % 3;
	int pos = microX * microY + microY;
	return pos;
}

std::array<Player, 9> UTTTBot::GetNextMicroBoard(std::array<std::array<Player, 9>, 9> array, int i, int i1)
{
	int x = i % 3;
	int y = i1 % 3;

	return GetMicroBoard(array, x, y);
}

int UTTTBot::RateByMacrogameFieldValue(Move &move, std::array<std::array<Player, 9>, 9> array)
{
	std::array<int, 9> potentialWins = {{3, 2, 3, 2, 4, 2, 3, 2, 3}}; // basically monte carlo in a single line for ttt

	// Send opponent to macro field with least amount of potential wins
	int microMove = GetMicroMove(move);
	int score = -potentialWins[microMove];

	return score;
}
