// main.cpp
// Aswin van Woudenberg

#include "uttt.h"
#include "utttbot.h"
#include "UTTTAI.h"

void test()
{
	State s = State();
	UTTTBot bot;
	UTTTAI::FindBestMove(s, 200);
}

int main()
{
	test();
	UTTTBot bot;
	bot.run();

	return 0;
}

