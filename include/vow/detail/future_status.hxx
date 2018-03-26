#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_STATUS
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_STATUS

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

enum class future_status
: unsigned char
{
    unlinked = 0,
    linked,
    ready,
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
