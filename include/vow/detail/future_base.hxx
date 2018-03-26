#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_BASE
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_BASE

#include <vow/detail/future_state.hxx>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
struct future_base
{
    virtual
    auto get_state()
    noexcept
    -> future_state<Value>
    = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
