#include "UTTT.h"
#include "TTT.h"

std::ostream &operator<<(std::ostream& os, const Player &p) {
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

std::ostream &operator<<(std::ostream& os, const State &s) {
	for (int r=0; r<9; r++) {
	    if(r % 3 == 0) os << std::endl;
		for (int c=0; c<9; c++) {
		    if(c % 3 == 0) os << "  ";
			os << s.board[r][c] << " ";
		}
		os << std::endl;
	}
	os << std::endl;

	for (int i = 0; i < 9; i++)
	    if(i % 3 == 0) os << std::endl << s.macroboard[i];
        else os << " " << s.macroboard[i];
	os << std::endl;

	return os;
}

std::ostream &operator<<(std::ostream& os, const Move &m) {
	os << m.x << " " << m.y;
	return os;
}

std::ostream &operator<<(std::ostream &os, const std::array<int, 9> &vals)
{
	os << "(";
	for(int i = 0; i < 8; i++)
		os << vals[i] << ", ";
	os << vals[8] << ")" << std::endl;
	return os;
}

State UTTT::doMove(const State &state, const Move &m)
{
	State result = state;

	int macroMove = m.x / 3 + 3 * (m.y / 3);

	int subX = m.x % 3;
	int subY = m.y % 3;
	int subMove = subX + subY * 3;

	Player player = state.turn;

	result.board[m.y][m.x] = player;
	result.subBoards[macroMove][subMove] = player;

	if(TTT::GetWinner(result.subBoards[macroMove]) == player)
		result.macroboard[macroMove] = player;

	if(result.macroboard[subMove] == Player::X || result.macroboard[subMove] == Player::O) {
        for (int i = 0; i < 9; i++)
            if (result.macroboard[i] != Player::X && result.macroboard[i] != Player::O)
                result.macroboard[i] = Player::Active;
    } else result.macroboard[subMove] = Player::Active;

	if(player == Player::X) result.round++;

	if(result.round >= 9)
		for(int w = 0; w < 8; w++)
			if(result.macroboard[TTT::wins[w][0]] == player
			&& result.macroboard[TTT::wins[w][0]] == result.macroboard[TTT::wins[w][1]]
			&& result.macroboard[TTT::wins[w][1]] == result.macroboard[TTT::wins[w][2]])
			{
				result.winner = player;
				break;
			}

	result.turn = state.turn == Player::O ? Player::X : Player::O;

	return result; 
}

std::vector<Move> UTTT::getMoves(const State &state)
{
	std::vector<Move> moves;
	if (state.winner == Player::None)
		for(int mb = 0; mb < state.macroboard.size(); mb++)
		if(state.macroboard[mb] == Player::Active)
			for(int md = 0; md < 9; md++)
				if(state.subBoards[mb][md] == Player::None)
					moves.push_back(Move{(mb % 3) * 3 + md % 3, mb / 3 * 3 + md / 3});

	return moves;
}

int UTTT::GetMicroMove(const Move &m)
{
	int microX = m.x % 3;
	int microY = m.y % 3;
	int pos = microX + microY * 3;
	return pos;
}
