#include "UTTTGame.h"

#include <algorithm>

#include "TTTGame.h"

std::ostream & operator << (std::ostream& os, const Player &p) {
	if (p == Player::None) {
		os << ".";
	} else if (p == Player::X) {
		os << "X";
	} else if (p == Player::O) {
		os << "O";
	} else if (p == Player::Active) {
		os << "*";
	}
	return os;
}

std::ostream & operator << (std::ostream& os, const Move &m) {
	os << m.x << " " << m.y;
	return os;
}

std::ostream & operator << (std::ostream &os, const std::array<int, 9> &vals)
{
	os << "(";
	for(int i = 0; i < 8; i++)
		os << vals[i] << ", ";
	os << vals[8] << ")" << std::endl;
	return os;
}

State UTTTGame::doMove(const State &state, const Move &m)
{
    // Copy previous state.
	State result = state;

	// The position of the TTT game where move is made.
	int macroMove = m.x / 3 + 3 * (m.y / 3);

	// The position of the move inside of previous TTT game.
	int subMove = m.x % 3 + (m.y % 3) * 3;

	// Get player making this move.
	Player player = state.turn;

	result.subBoards[macroMove][subMove] = player;

	if(TTTGame::GetWinner(result.subBoards[macroMove]) == player)
		result.macroBoard[macroMove] = player;

	// If next player would've been sent to a finished TTT board...
	if(result.macroBoard[subMove] == Player::X || result.macroBoard[subMove] == Player::O ||
	std::find(result.subBoards[subMove].begin(), result.subBoards[subMove].end(), Player::None) == result.subBoards[subMove].end()) {
	    // ... Set all unfinished sub-boards active.
        for (int i = 0; i < 9; i++)
            if (result.macroBoard[i] == Player::None) result.macroBoard[i] = Player::Active;
    } else // Set the board represented at player's sub-move position as active.
	{
		for(int s = 0; s < 9; s++)
			if(result.macroBoard[s] == Player::Active)
				result.macroBoard[s] = Player::None;
		result.macroBoard[subMove] = Player::Active;
	}

	// Increment round after second player to move made a move.
	if(player == Player::X) result.round++;

	// Check for a winner, don't bother before 9 rounds as a win needs 9 moves minimum.
	if(result.round >= 9)
		for (auto & win : TTTGame::wins)
            if(result.macroBoard[win[0]] == player // Only the player making the move can be the winner of the resulting state.
			&& result.macroBoard[win[0]] == result.macroBoard[win[1]]
			&& result.macroBoard[win[1]] == result.macroBoard[win[2]])
			{
				result.winner = player;
				break;
			}

	// TODO: update subGamesWinnable and Gamewinnableby in UTTTBot Riddles hook

	// Only try to update potential wins if opponent is affected (as only opponent can loose a potential win when bot is on move)
	if(state.subGamesWinnableBy[macroMove] == Player::Both || state.subGamesWinnableBy[macroMove] == state.opponent) {
		Player potentialSubWinner = TTTGame::IsWinnableForPlayer(result.subBoards[macroMove]);
		if(potentialSubWinner != result.subGamesWinnableBy[macroMove]) { // Opponent can no longer win sub-board
			result.subGamesWinnableBy[macroMove] = potentialSubWinner;
			Player canWin = Player::None;
			for (auto win : TTTGame::wins)
			{
				// If changed macro board is part of this win
				if(win[0] == macroMove) {
					if((state.subGamesWinnableBy[win[1]] == state.opponent || state.subGamesWinnableBy[win[1]] == Player::Both)
					&& (state.subGamesWinnableBy[win[2]] == state.opponent || state.subGamesWinnableBy[win[2]] == Player::Both))
						result.winsOpp--;
				}
				else if(win[1] == macroMove) {
					if((state.subGamesWinnableBy[win[0]] == state.opponent || state.subGamesWinnableBy[win[0]] == Player::Both)
					   && (state.subGamesWinnableBy[win[2]] == state.opponent || state.subGamesWinnableBy[win[2]] == Player::Both))
						result.winsOpp--;
				}
				else if(win[2] == macroMove) {
					if((state.subGamesWinnableBy[win[1]] == state.opponent || state.subGamesWinnableBy[win[1]] == Player::Both)
					   && (state.subGamesWinnableBy[win[0]] == state.opponent || state.subGamesWinnableBy[win[0]] == Player::Both))
						result.winsOpp--;
				}
			}

		}
	}

	// Switch turns.
	result.turn = state.turn == Player::O ? Player::X : Player::O;

	return result; 
}

std::vector<Move> UTTTGame::getMoves(const State &state)
{
	std::vector<Move> moves;
	if (state.winner != Player::None) return moves;

    for(int mb = 0; mb < state.macroBoard.size(); mb++)
    {
        if (state.macroBoard[mb] != Player::Active) continue;

        int deltaMx = (mb % 3) * 3;
        int deltaMy = mb / 3 * 3;

        for (int md = 0; md < 9; md++)
            if (state.subBoards[mb][md] == Player::None)
                moves.push_back(Move{deltaMx + md % 3, deltaMy + md / 3});
    }

	return moves;
}
