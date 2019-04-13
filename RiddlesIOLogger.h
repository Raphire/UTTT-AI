#ifndef RIDDLESIOLOGGER_H
#define RIDDLESIOLOGGER_H

#include <string>
#include <vector>
#include "UTTTGame.h"

const int VISIBLE_LOG_LEVEL = 5;

enum LogLevel {
    Critical = 0,
    Error = 1,
    Waring = 2,
    Info = 3,
    Debug = 4,
    Trace = 5
};

enum UTTTAIPredefinedMessageType {
    BEGIN_OF_SEARCH,
    STATE_ASSESSMENT_DONE,
    ERROR_DO_MOVE_ON_FINISHED_GAME,
    BEGIN_ELIMINATION_OF_MOVES,
    SELECTIONSTAGE_PASSED_SUMMARY,
    SINGLE_BEST_MOVE_FOUND,
    MULTIPLE_BEST_MOVES_FOUND,
    MINIMAX_SEARCH_FINISHED,
    MINIMAX_SEARCH_FINISHED_ALL_EVALUATED,
    ERROR_BOT_RECEIVED_UNKNOWN_INPUT,
    DEBUG_REPLAY_ROUND,
    MOVE_IRRELEVANT
};

/// Class used to help with logging on the Riddles.io platform
class RiddlesIOLogger {
public:
    static void Log(UTTTAIPredefinedMessageType, std::vector<std::string> args);
    static void Log(std::string message, LogLevel level);

    static std::string MovesToString(std::vector<Move> vector);
};


#endif
