#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_PROMISE_STATUS
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_PROMISE_STATUS

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

enum class promise_status
: unsigned char
{
    unlinked = 0,
    linked,
    ready,
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
