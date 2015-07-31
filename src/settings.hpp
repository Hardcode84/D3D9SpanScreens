#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>
#include <vector>
#include "simple_logger/logger.hpp"

class settings
{
public:
    settings(const std::string& filename);

    logger::LogLevel logLevel() const { return mLogLevel; }
    std::string      logFile()  const { return mLogFile; }
    bool restoreStates()        const { return mRestoreStates; }
    bool isHorizontal()         const { return mIsHorizontal; }
    std::vector<int> getScreensMapping() const { return mScreensMapping; }
private:
    logger::LogLevel mLogLevel;
    std::string mLogFile;
    bool mRestoreStates;
    bool mIsHorizontal;
    std::vector<int> mScreensMapping;
};

#endif // SETTINGS_HPP
