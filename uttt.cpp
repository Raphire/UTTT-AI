// uttt.cpp
// Aswin van Woudenberg

#include "uttt.h"

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
		for (int c=0; c<9; c++) {
			os << s.board[r][c];
		}
		os << std::endl;
	}
	os << std::endl;
	for (int r=0; r<3; r++) {
		for (int c=0; c<3; c++) {
			os << s.macroboard[r][c];
		}
		os << std::endl;
	}
	return os;
}

std::ostream &operator<<(std::ostream& os, const Move &m) {
	os << m.x << " " << m.y;
	return os;
}

Player uttt::getCurrentPlayer(const State &state)
{
	int countX = 0;
	int countO = 0;
	for (int r=0; r<9; r++) {
		for (int c=0; c<9; c++) {
			if (state.board[r][c] == Player::X) countX++;
			else if (state.board[r][c] == Player::O) countO++;
		}
	}
	return (countX > countO ? Player::O : Player::X); 
}

Player uttt::getWinner(const State &state, int row, int col)
{
	for (int r=0; r<3; r++)
		if (state.board[row*3+r][col*3] == state.board[row*3+r][col*3+1] && 
				state.board[row*3+r][col*3+1] == state.board[row*3+r][col*3+2] && 
				state.board[row*3+r][col*3+2] != Player::None) 
			return state.board[row*3+r][col*3];
	for (int c=0; c<3; c++)
		if (state.board[row*3][col*3+c] == state.board[row*3+1][col*3+c] && 
				state.board[row*3+1][col*3+c] == state.board[row*3+2][col*3+c] && 
				state.board[row*3][col*3+c] != Player::None) 
			return state.board[row*3][col*3+c];
	if (state.board[row*3][col*3] == state.board[row*3+1][col*3+1] && 
			state.board[row*3+1][col*3+1] == state.board[row*3+2][col*3+2] && 
			state.board[row*3][col*3] != Player::None) 
		return state.board[row*3][col*3];
	if (state.board[row*3][col*3+2] == state.board[row*3+1][col*3+1] && 
			state.board[row*3+1][col*3+1] == state.board[row*3+2][col*3] && 
			state.board[row*3][col*3+2] != Player::None) 
		return state.board[row*3][col*3+2];
	for (int r=0; r<3; r++)
		for (int c=0; c<3; c++)
			if (state.board[row*3+r][col*3+c] == Player::None)
				return Player::Active;
	return Player::None;
}

State uttt::doMove(const State &state, const Move &m)
{
	State result = state;

	if (state.macroboard[m.y/3][m.x/3] != Player::Active) {
		return result; // Invalid move
	}

	result.board[m.y][m.x] = getCurrentPlayer(state);
	for (int r=0; r<3; r++) {
		for (int c=0; c<3; c++) {
			result.macroboard[r][c] = getWinner(result, r, c);
		}
	}

	int empty = 0;
	for (int r=0; r<9; r++)
		for (int c=0; c<9; c++)
			if (result.board[r][c] == Player::None)
				empty++;
	bool stillPlaying = empty > 0 && getWinner(result) == Player::None;
	if (result.macroboard[m.y%3][m.x%3] == Player::Active)
		for (int r=0; r<3; r++)
			for (int c=0; c<3; c++)
				if ((r!=m.y%3 || c!=m.x%3 || !stillPlaying) && result.macroboard[r][c] == Player::Active)
					result.macroboard[r][c] = Player::None;

	return result; 
}

Player uttt::getWinner(const State &state)
{
	for (int r=0; r<3; r++)
		if (state.macroboard[r][0] == state.macroboard[r][1] && 
				state.macroboard[r][1] == state.macroboard[r][2] && 
				state.macroboard[r][2] != Player::None && 
				state.macroboard[r][2] != Player::Active) 
			return state.macroboard[r][2];
	for (int c=0; c<3; c++)
		if (state.macroboard[0][c] == state.macroboard[1][c] && 
				state.macroboard[1][c] == state.macroboard[2][c] && 
				state.macroboard[2][c] != Player::None && 
				state.macroboard[2][c] != Player::Active) 
			return state.macroboard[2][c];
	if (state.macroboard[0][0] == state.macroboard[1][1] && state.macroboard[1][1] == state.macroboard[2][2] && 
			state.macroboard[0][0] != Player::None && state.macroboard[0][0] != Player::Active) 
		return state.macroboard[0][0];
	if (state.macroboard[0][2] == state.macroboard[1][1] && state.macroboard[1][1] == state.macroboard[2][0] && 
			state.macroboard[0][2] != Player::None && state.macroboard[0][2] != Player::Active) 
		return state.macroboard[0][2];
	return Player::None;
}

std::vector<Move> uttt::getMoves(const State &state)
{
	std::vector<Move> moves;
	if (getWinner(state) == Player::None) {
		for (int r=0; r<9; r++) {
			for (int c=0; c<9; c++) {
				if (state.macroboard[r/3][c/3] == Player::Active && state.board[r][c] == Player::None) {
					moves.push_back(Move{c,r});
				}
			}
		}
	}
	return moves;
}

std::array<Player, 9> uttt::GetSubBoard(std::array<std::array<Player, 9>, 9> array, int i, int i1)
{
	std::array<Player, 9> b = std::array<Player, 9>();
	for(int x = 0; x < 3; x++) for(int y = 0; y < 3; y++) b[x*3 + y] = array[i*3 + x][i1*3 + y];
	return b;
}

int uttt::GetMicroMove(const Move &m)
{
	int microX = m.x % 3;
	int microY = m.y % 3;
	int pos = microX + microY * 3;
	return pos;
}

std::array<Player, 9> uttt::GetNextSubBoard(std::array<std::array<Player, 9>, 9> array, int i, int i1)
{
    int x = i % 3;
    int y = i1 % 3;

    return GetSubBoard(array, x, y);
}

std::array<std::array<Player, 9>, 9> uttt::GetSubBoards(std::array<std::array<Player, 9>, 9> macroBoard)
{
	std::array<std::array<Player, 9>, 9> subBoards;
	for(int x = 0; x < 9; x++)
		for(int y = 0; y < 9; y++)
		{
			int boardNo = y / 3 + (x / 3) * 3;
			int boardPos = y % 3 + (x % 3) * 3;
			subBoards[boardNo][boardPos] = macroBoard[x][y];
		}
	return subBoards;
}
