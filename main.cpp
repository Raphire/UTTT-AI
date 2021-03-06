// main.cpp
// Jeffrey Drost

#include "utttbot.h"
#include <vector>

void test()
{
    UTTTBot bot;
    std::vector<std::string> input = {
            "settings player_names player0,player1",
            "settings your_bot player1",
            "settings timebank 5000",
            "settings time_per_move 200",
            "settings your_botid 1",
            "update game round 1",
            "update game field .,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.",
            "update game macroboard -1,-1,-1,-1,-1,-1,-1,-1,-1",
            //"update game field .,.,.,.,.,1,.,.,.,0,.,0,.,.,1,0,.,0,.,.,1,.,.,1,.,.,.,1,.,.,.,0,.,.,0,.,1,.,.,.,.,.,.,.,.,1,0,.,.,1,1,1,1,1,.,0,.,1,0,1,0,.,.,0,.,.,.,.,0,.,0,.,1,0,.,0,1,.,1,0,0",
            //"update game macroboard .,1,.,1,.,1,-1,.,0",
            "action move 10000"
    };

    for(int i = 0; i < input.size(); i++)
        bot.input(input[i]);
}

int main() {
    //test();

	UTTTBot bot;
	bot.run();

	return 0;
}
