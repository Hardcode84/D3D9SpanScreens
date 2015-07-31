#ifndef HELP_FUNCS_HPP
#define HELP_FUNCS_HPP

#include <algorithm>
#include <vector>

namespace help_funcs
{

template<typename T>
T bound(const T& min, const T& val, const T& max)
{
    return std::max(std::min(val, max), min);
}

template<typename T>
void fill_rect(void* data,
               int x, int y,
               int width, int height,
               size_t pitch,
               const T& value)
{
    unsigned char* p = reinterpret_cast<unsigned char*>(data);
    unsigned char* start = p + y * pitch + x * sizeof(T);
    for(int i = 0; i < height; ++i)
    {
        std::fill_n(reinterpret_cast<T*>(start), width, value);
        start += pitch;
    }
}

template<typename DstT, typename SrcT>
DstT union_cast(const SrcT& val)
{
    union
    {
        SrcT s;
        DstT d;
    } u;
    u.s = val;
    return u.d;
}

bool patch_address(void* addr, const void* newData, size_t dataSize);

template<typename T>
bool patch_address(void* var, const T& newVar)
{
    return patch_address(var, &newVar, sizeof(T));
}

template<typename StrT, typename CharT>
std::vector<StrT> split(const StrT& str, const CharT& sep, bool skipEmpty = true)
{
    std::vector<StrT> ret;
    ret.push_back(StrT());
    for(const auto ch: str)
    {
        if(ch == sep)
        {
            if(!skipEmpty || !ret.back().empty())
            {
                ret.push_back(StrT());
            }
        }
        else
        {
            ret.back() += ch;
        }
    }
    if(skipEmpty && ret.back().empty())
    {
        ret.pop_back();
    }
    return ret;
}

}

#endif // HELP_FUNCS_HPP
