#include "common.hpp"

#include <dxerr9.h>

winerr_wrapper getWinError()
{
    return winerr_wrapper(GetLastError());
}

std::string getWinErrorFromHr(HRESULT hr)
{
    return ::DXGetErrorString9A(hr);
}

