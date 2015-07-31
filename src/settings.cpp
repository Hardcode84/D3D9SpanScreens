#include "settings.hpp"

#include "tools/help_funcs.hpp"
#include "tools/ini_file.hpp"

settings::settings(const std::string& filename)
{
    const ini_file file(filename);
    mLogLevel         = (logger::LogLevel)file.getInt("main","log_level",logger::logDEBUG);
    mLogFile          = file.getString("main","log_file","debug.log");
    mRestoreStates    = file.getInt("main","restore_states", 1);
    mIsHorizontal     = file.getInt("main","is_horizontal", 1);
    const auto strs   = help_funcs::split(file.getString("main","screens_mapping",std::string()), ',', true);
    mScreensMapping.resize(strs.size());
    std::transform(strs.begin(), strs.end(), mScreensMapping.begin(), [](const std::string& str)
    {
        return std::atoi(str.c_str());
    });
}
