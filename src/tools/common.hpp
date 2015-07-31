#ifndef COMMON_HPP
#define COMMON_HPP

#include <windows.h>
#include <atomic>
#include <algorithm>

#include "../simple_logger/logger.hpp"
#include "com_ptr.hpp"

extern logger* gLog;

struct winerr_wrapper
{
    DWORD err;
    winerr_wrapper(DWORD e):err(e) {}
};

winerr_wrapper getWinError();
std::string getWinErrorFromHr(HRESULT hr);

template<typename STREAM>
STREAM& operator<<(STREAM& s, const GUID& g)
{
    s << g.Data1 << '-' << g.Data2 << '-' << g.Data3;
    for(const auto c: g.Data4)
    {
        s << '-';
        s << (int)c;
    }
    return s;
}

template<typename STREAM>
STREAM& operator<<(STREAM& s, const RECT& rc)
{
    s << "RECT: left: " << rc.left << " right: " << rc.right << " top: " << rc.top << " bottom: " << rc.bottom;
    return s;
}

template<typename STREAM>
STREAM& operator<<(STREAM& s, const winerr_wrapper& e)
{
    LPTSTR lpMsgBuf = nullptr;
    const DWORD size = ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            e.err,
            MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0, nullptr);
    for(DWORD i = 0; i < size; ++i)
    {
        if(('\r' == lpMsgBuf[i]) || ('\n' == lpMsgBuf[i]))
        {
            lpMsgBuf[i] = ' ';
        }
    }
    s << '\"' << lpMsgBuf << "\", code " << e.err;
    LocalFree(lpMsgBuf);
    return s;
}

#define LOG_ERROR()   WRITE_LOG(*gLog,logger::logERROR)
#define LOG_WARNING() WRITE_LOG(*gLog,logger::logWARNING)
#define LOG_INFO()    WRITE_LOG(*gLog,logger::logINFO)
#define LOG_DEBUG()   WRITE_LOG(*gLog,logger::logDEBUG)

#ifdef __GNUC__
#define FUNC_NAME __PRETTY_FUNCTION__
#else
#define FUNC_NAME __func__
#endif

#define MY_ERROR(text) do { LOG_ERROR() << #text; abort(); } while(0)

#define MY_STR(expr) #expr
#define MY_STR2(expr) MY_STR(expr)

#if defined(__GNUC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 5)
    #define MY_ASSUME(cond) do { if(!(cond)) __builtin_unreachable(); } while(0)
#elif defined(_MSC_VER)
    #define MY_ASSUME(cond) __assume(cond)
#else
    #define MY_ASSUME(cond)
#endif

#if defined(_DEBUG) || defined(DEBUG)
    #define MY_ASSERT(expr) if(!(expr)) MY_ERROR(__FILE__ " " MY_STR2(__LINE__) ": Assertion failed: " #expr)
#else
    #define MY_ASSERT(expr) MY_ASSUME(expr)
#endif

#define WRITE_VAR(var)       LOG_INFO()  << #var << "=\"" << var << '\"'
#define WRITE_VAR_DEBUG(var) LOG_DEBUG() << #var << "=\"" << var << '\"'
#define WRITE_FLAG(var,flag) if((var) & (flag)) do{ LOG_INFO() << #var ": \"" #flag << "\" set"; }while(false)
#define WRITE_THIS() WRITE_VAR_DEBUG(this)

#define LOG_STATIC_FUNCTION() LOG_SCOPE(*gLog,logger::logDEBUG,(FUNC_NAME));

#define LOG_FUNCTION() LOG_SCOPE(*gLog,logger::logDEBUG,(FUNC_NAME)," this=",this);
#define LOG_FUNCTION_E() LOG_SCOPE(*gLog,logger::logINFO,(FUNC_NAME)," this=",this); \
    LOG_ERROR() << (FUNC_NAME) << " not implemented;"; return E_NOTIMPL;

#define CHECK_PARAM(param) do{ if(!(param)) { LOG_ERROR() << "Param \"" #param "\" failed"; return E_INVALIDARG; } }while(false)

#define CHECK_COM_METHOD(interface,method, ...) \
if(FAILED(hr = interface->method(__VA_ARGS__))) { \
    LOG_ERROR() << #method " failed: " << getWinErrorFromHr(hr); \
    return hr; }

#define CHECK_STAIC_COM_METHOD(method, ...) \
if(FAILED(hr = method(__VA_ARGS__))) { \
    LOG_ERROR() << #method " failed: " << getWinErrorFromHr(hr); \
    return hr; }

#if defined(DEBUG) || defined(_DEBUG)
    #define CHECK_COM_METHOD_DEBUG(interface,method, ...) CHECK_COM_METHOD(interface,method,__VA_ARGS__);
#else
    #define CHECK_COM_METHOD_DEBUG(interface,method, ...) hr = interface->method(__VA_ARGS__)
#endif

template<typename BaseT, typename SrcT>
BaseT* adjust_pointer(SrcT* src)
{
    if(nullptr == src) return nullptr;
    const BaseT* dummy1 = nullptr;
    const SrcT*  dummy2 = static_cast<const SrcT*>(dummy1);
    const auto offset = reinterpret_cast<const char*>(dummy2) - reinterpret_cast<const char*>(dummy1);
    char* p = reinterpret_cast<char*>(src);
    p -= offset;
    return reinterpret_cast<BaseT*>(p);
}

#endif // COMMON_HPP
