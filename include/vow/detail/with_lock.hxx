#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_WITH_LOCK
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_WITH_LOCK

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

struct with_lock_t
{
    explicit
    with_lock_t()
    = default;
};

inline constexpr
with_lock_t with_lock{};

///////////////////////////////////////////////////////////////////////////////
}

#endif
