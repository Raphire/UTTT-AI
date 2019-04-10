#include "uttt.h"
#include "utttbot.h"
#include "UTTTAI.h"

void test()
{
	UTTTBot bot;
	std::vector<std::string> input = {
		"settings player_names player0,player1",
		"settings your_bot player0",
		"settings timebank 10000",
		"settings time_per_move 100",
		"settings your_botid 0",
		"update game round 17",
		"update game field .,0,.,1,.,1,.,0,1,.,0,.,.,.,1,0,.,.,.,0,.,.,.,1,.,0,.,.,.,1,1,0,.,.,0,.,.,0,.,.,.,.,.,.,.,1,.,1,1,.,.,1,.,.,.,.,0,1,.,.,.,.,1,0,.,0,1,.,.,0,.,.,1,0,0,1,.,.,0,.,.",
		"update game macroboard 0,1,-1,.,.,.,0,1,.",
		"action move 9706"
	};
	for(int i = 0; i < input.size(); i++)
	    bot.input(input[i]);
}

int main()
{
	test();
	//UTTTBot bot;
	//bot.run();

	return 0;
}
