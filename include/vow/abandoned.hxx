#ifndef VOW_DETAIL_INCLUDE_GUARD_ABANDONED
#define VOW_DETAIL_INCLUDE_GUARD_ABANDONED

#include <exception>

namespace vow {
///////////////////////////////////////////////////////////////////////////////

struct abandoned
: std::exception
{
    auto what() const
    noexcept
    -> char const*
    override
    { return "vow::abandoned"; }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
