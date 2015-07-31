#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>
#include <fstream>
#include <mutex>
#include <tuple>

class logger final
{
public:
    friend class log_entry;
    enum LogLevel
    {
        logDISABLE = -1,
        logERROR = 0,
        logWARNING,
        logINFO,
        logDEBUG
    };

    logger(LogLevel level = logDEBUG, bool writeToCout = true, bool isUnbuffered = false);
    ~logger();

    LogLevel level() const { return mLevel; }

    void setFile(const std::string& filename, bool truncate = true);

    bool isActive() const { return mLevel >= 0 && (mWriteToCout || mFileStream); }

private:
    std::stringstream& get() { return mStream; }
    void lock()   { mLock.lock(); }
    void unlock() { mLock.unlock(); }
    void flush(bool force);

    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    LogLevel mLevel;
    bool mWriteToCout;
    bool mIsUnbuffered;

    std::mutex mLock;
    std::stringstream mStream;
    std::ofstream mFileStream;
};

class log_entry final
{
    logger& mLogger;
    const logger::LogLevel mLevel;
public:
    log_entry(logger& l, logger::LogLevel level);
    ~log_entry();

    std::stringstream& get() { return mLogger.get(); }
};

#define WRITE_LOG(log, lev) if((lev) > (log).level() || !(log).isActive()); \
else log_entry((log),(lev)).get()

template<size_t N>
struct tuple_printer
{
    template<typename STREAM, typename TUPLE>
    static void print(STREAM& s, const TUPLE& t)
    {
        tuple_printer<N-1>::print(s, t);
        s << std::get<N-1>(t);
    }
};

template<>
struct tuple_printer<0>
{
    template<typename STREAM, typename TUPLE>
    static void print(STREAM&, const TUPLE&)
    {
    }
};


template<typename STREAM, typename... Args>
STREAM& operator<<(STREAM& s, const std::tuple<Args...>& t)
{
    tuple_printer<sizeof...(Args)>::print(s, t);
    return s;
}


template<typename T>
class scoped_helper;

template<>
class scoped_helper<char*>
{
    logger& mLogger;
    const char* mVal;
    const logger::LogLevel mLevel;
public:
    template<typename... Args>
    inline scoped_helper(logger& log, logger::LogLevel level, const char* val, Args... args):
        mLogger(log),
        mVal(val),
        mLevel(level)
    {
        WRITE_LOG(mLogger, mLevel) << mVal << std::tie(args...) << ": enter";
    }

    inline ~scoped_helper()
    {
        WRITE_LOG(mLogger, mLevel) << mVal << ": exit";
    }
};

template<size_t N>
class scoped_helper<char[N]>
{
    logger& mLogger;
    const char* mVal;
    const logger::LogLevel mLevel;
public:
    template<typename... Args>
    inline scoped_helper(logger& log, logger::LogLevel level, const char val[N], Args... args):
        mLogger(log),
        mVal(val),
        mLevel(level)
    {
        WRITE_LOG(mLogger, mLevel) << mVal << std::tie(args...) << " enter";
    }

    inline ~scoped_helper()
    {
        WRITE_LOG(mLogger, mLevel) << mVal << " exit";
    }
};

template<>
class scoped_helper<std::string>
{
    logger& mLogger;
    const std::string mVal;
    const logger::LogLevel mLevel;
public:
    template<typename... Args>
    inline scoped_helper(logger& log, logger::LogLevel level, const std::string& val, Args... args):
        mLogger(log),
        mVal(val),
        mLevel(level)
    {
        WRITE_LOG(mLogger, mLevel) << mVal << std::tie(args...) << ": enter";
    }

    inline ~scoped_helper()
    {
        WRITE_LOG(mLogger, mLevel) << mVal << ": exit";
    }
};

template<typename T, typename... Args>
scoped_helper<T> create_scoped_helper(logger& log, logger::LogLevel level,const T& val, Args... args)
{
    return scoped_helper<T>(log, level, val, args...);
}

#define LOG_UNIQ_NAME(line) log_scope_guard_##line
#define LOG_UNIQ_NAME2(line) LOG_UNIQ_NAME(line)
#define LOG_SCOPE(log,level,...) const auto LOG_UNIQ_NAME2(__LINE__) = create_scoped_helper(log,level,__VA_ARGS__)

#endif // LOGGER_HPP
