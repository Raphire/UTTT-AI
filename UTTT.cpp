#include "UTTT.h"

#include "TTT.h"

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

State UTTT::doMove(const State &state, const Move &m)
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

	if(TTT::GetWinner(result.subBoards[macroMove]) == player)
		result.macroBoard[macroMove] = player;

	// If next player would've been sent to a finished TTT board...
	if(result.macroBoard[subMove] == Player::X || result.macroBoard[subMove] == Player::O) {
	    // ... Set all unfinished sub-boards active.
        for (int i = 0; i < 9; i++)
            if (result.macroBoard[i] == Player::None) result.macroBoard[i] = Player::Active;
    } else // Set the board represented at player's sub-move position as active.
        result.macroBoard[subMove] = Player::Active;

	// Increment round after second player to move made a move.
	if(player == Player::X) result.round++;

	// Check for a winner, don't bother before 9 rounds as a win needs 9 moves minimum.
	if(result.round >= 9)
		for (auto & win : TTT::wins)
            if(result.macroBoard[win[0]] == player // Only the player making the move can be the winner of the resulting state.
			&& result.macroBoard[win[0]] == result.macroBoard[win[1]]
			&& result.macroBoard[win[1]] == result.macroBoard[win[2]])
			{
				result.winner = player;
				break;
			}

	// Switch turns.
	result.turn = state.turn == Player::O ? Player::X : Player::O;

	return result; 
}

std::vector<Move> UTTT::getMoves(const State &state)
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
