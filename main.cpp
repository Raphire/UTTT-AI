#include "UTTTBot.h"
#include "UTTTAI.h"

void test()
{
	UTTTBot bot;
	std::vector<std::string> lines;
	lines = {"settings player_names player0,player1", "settings your_bot player0", "settings timebank 10000", "settings time_per_move 100", "settings your_botid 0", "update game round 11", "update game field .,.,.,1,.,1,.,.,.,0,.,0,.,.,.,0,0,0,.,.,.,.,.,.,.,.,.,.,0,1,.,0,.,1,.,1,.,1,1,.,.,.,1,.,.,.,.,.,.,0,1,.,.,.,.,.,.,.,.,.,.,.,.,.,.,0,.,.,.,0,.,.,.,.,.,1,.,.,.,.,.", "update game macroboard .,.,0,.,.,-1,.,.,.", "action move 9869"};

	for (auto & i : lines)
	    bot.input(i);
}

int main()
{
    //test();
	UTTTBot bot;
	bot.run();

	return 0;
}
