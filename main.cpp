#include "UTTTBot.h"
#include "UTTTAI.h"

void test()
{
	UTTTBot bot;
	std::vector<std::string> lines = {"settings player_names player0,player1", "settings your_bot player0", "settings timebank 5000", "settings time_per_move 100", "settings your_botid 0", "update game round 22", "update game field .,0,.,1,.,1,.,1,1,.,1,.,.,0,0,0,.,0,0,0,1,.,.,1,1,1,1,1,1,0,.,.,.,.,.,.,1,0,.,0,0,0,1,1,1,1,.,.,.,.,.,.,.,.,0,0,.,1,.,.,.,0,.,0,.,.,0,0,.,.,0,.,1,0,.,1,1,.,.,0,.", "update game macroboard -1,.,1,1,0,1,.,.,0", "action move 8523"};

	for (auto & i : lines)
	    bot.input(i);

	/*
	int time = 0;
	int samples = 0;

	for(int i = 0; i < 10000; i++) {
		auto startTime = std::chrono::steady_clock::now();

		UTTTAI::GetChildStates(bot.getCurrentGameState());

		time += std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::steady_clock::now() - startTime).count();

		samples++;
	}
    std::cerr << "Time: " << ((double) time / samples) << std::endl;
    */
}

int main()
{
    test();
	UTTTBot bot;
	bot.run();

	return 0;
}
