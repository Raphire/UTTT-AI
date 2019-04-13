#include "UTTTBot.h"
#include "UTTTAI.h"

void test()
{
	UTTTBot bot;
	std::vector<std::string> lines;

	lines = {"settings player_names player0,player1", "settings your_bot player1", "settings timebank 10000", "settings time_per_move 100", "settings your_botid 1", "update game round 20", "update game field 1,.,0,0,.,.,.,1,.,.,1,1,0,.,.,.,1,.,.,.,0,0,.,.,.,1,.,.,.,.,1,1,0,0,.,.,1,.,0,.,0,.,.,0,1,0,.,0,0,.,.,.,.,0,0,.,.,.,1,0,0,.,.,1,.,1,.,.,.,1,0,.,1,0,1,.,.,1,1,0,1", "update game macroboard -1,0,1,.,0,0,.,.,.", "action move 9498"};

	for (auto & i : lines)
	    bot.input(i);
}

int main()
{
    test();
	UTTTBot bot;
	bot.run();

	return 0;
}
