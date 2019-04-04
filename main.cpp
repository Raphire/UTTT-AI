// main.cpp
// Aswin van Woudenberg

#include "utttbot.h"
#include "ttt.h"
#include <vector>

void test()
{
	State s = State();
	UTTTBot bot;
	bot.findBestMove(s, 10000);
}

int main()
{
	test();

	UTTTBot bot;
	bot.run();

	return 0;
}

