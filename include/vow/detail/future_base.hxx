#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_BASE
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_BASE

#include <vow/result.hxx>
#include <vow/detail/future_state.hxx>

namespace vow {
    template <typename Value>
    class promise;
}

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
struct future_base
{

private:
    friend class promise<Value>;

    virtual
    auto get_state()
    noexcept
    -> future_state<Value>
    = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
