#include "help_funcs.hpp"

#include "common.hpp"

namespace help_funcs
{
bool patch_address(void* addr, const void* newData, size_t dataSize)
{
    DWORD oldProtect;
    if(!::VirtualProtect(addr, dataSize, PAGE_READWRITE, &oldProtect))
    {
        LOG_ERROR() << "VirtualProtect failed: " << getWinError();
        return false;
    }
    memcpy(addr, newData, dataSize);
    if(!::VirtualProtect(addr, dataSize, oldProtect, &oldProtect))
    {
        LOG_ERROR() << "VirtualProtect failed: " << getWinError();
        return false;
    }
    return true;
}
}
