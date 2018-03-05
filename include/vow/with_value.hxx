#ifndef VOW_DETAIL_INCLUDE_GUARD_WITH_VALUE
#define VOW_DETAIL_INCLUDE_GUARD_WITH_VALUE

namespace vow {
///////////////////////////////////////////////////////////////////////////////

struct with_value_t
{
    explicit
    with_value_t()
    = default;
};

inline constexpr
with_value_t with_value{};

///////////////////////////////////////////////////////////////////////////////
}

#endif
