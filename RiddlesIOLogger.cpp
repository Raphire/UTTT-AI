#include "RiddlesIOLogger.h"

#include <sstream>

void RiddlesIOLogger::Log(std::string message, LogLevel level)
{
    if(level <= VISIBLE_LOG_LEVEL)
        std::cerr << message << std::endl;
}

void RiddlesIOLogger::Log(UTTTAIPredefinedMessageType type, std::vector<std::string> args)
{
    switch(type)
    {
        case BEGIN_OF_SEARCH:
            Log("Starting move search for round: " + args[0] + ", Turn #" + args[1] + "." , LogLevel::Info);
            Log("---------------------------------------------------------------------------------------", LogLevel::Info);
            break;
        case STATE_ASSESSMENT_DONE:
            Log("Assessment of state completed, match should be over within " + args[0] + " moves. \n", LogLevel::Info);
            break;
        case BEGIN_ELIMINATION_OF_MOVES:
            Log("Starting elimination of moves. (" + args[0] + " ms)", LogLevel::Info);
            break;
        case MULTIPLE_BEST_MOVES_FOUND:
            Log("Multiple optimal moves have been found. Selecting one randomly... (" + args[0] + " ms)", LogLevel::Info);
            Log("Moves: " + args[1], LogLevel::Info);
            break;
        case SELECTIONSTAGE_PASSED_SUMMARY:
            Log("Stage #" + args[0] + " (" + args[1] + ") eliminated " + args[2] + " of " + args[3] + " moves. (" + args[4] + " ms)", LogLevel::Info);
            Log("Highest rating moves are rated " + args[5] + ".", LogLevel::Info);
            Log("Remaining viable moves: " + args[6] + "\n", LogLevel::Info);
            break;
        case SINGLE_BEST_MOVE_FOUND:
            Log("Found a single best move (" + args[0] + "), search done.", LogLevel::Info);
            break;
        case MINIMAX_SEARCH_FINISHED_ALL_EVALUATED:
            Log("MiniMax successfully traversed entire game-tree traversing " + args[0] + " game states @ " + args[1] + " States/ms, Search depth: " + args[2], LogLevel::Info);
            break;
        case MINIMAX_SEARCH_FINISHED:
            Log("MiniMax finished after abort (Search depth: " + args[2] + ", " + args[0] + " game states traversed @ " + args[1] + " States/ms).", LogLevel::Info);
            break;
        case ERROR_BOT_RECEIVED_UNKNOWN_INPUT:
            Log("Unknown command: " + args[0], LogLevel::Error);
            break;
        case ERROR_DO_MOVE_ON_FINISHED_GAME:
            Log("Bot was asked to find a move on a finished match.", LogLevel::Error);
            break;
        case DEBUG_REPLAY_ROUND:
            Log("---------------------------------------------------------------------------------------", LogLevel::Info);
            Log("Copy this to repeat this round locally: ", LogLevel::Info);
            Log(args[0], LogLevel::Info);
            Log("---------------------------------------------------------------------------------------", LogLevel::Info);
            break;
        case MOVE_IRRELEVANT:
            Log("Neither player can still win this match, i shouldn't risk a timeout by calculating anything more.", LogLevel::Info);
            break;
        default:
            Log("Attempt to log unspecified message type.", LogLevel::Info);
            break;
    }
}

std::string RiddlesIOLogger::MovesToString(std::vector<Move> m)
{
    std::stringstream ss;
    if(m.empty()) return ss.str();

    ss << "(X: " << m[0].x << " Y: " << m[0].y << ")";

    for(int mi = 1; mi < m.size(); mi++)
        ss << ", (X: " << m[mi].x << " Y: " << m[mi].y << ")";

    return ss.str();
}
