#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_MOVE_CONSTRUCTIBLE_IF
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_MOVE_CONSTRUCTIBLE_IF

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <bool>
struct move_constructible_if {};

template <>
struct move_constructible_if<false>
{
    move_constructible_if()
    = default;

    move_constructible_if(move_constructible_if&&)
    = delete;
    move_constructible_if(move_constructible_if const&)
    = default;

    auto operator=(move_constructible_if&&)
    -> move_constructible_if&
    = default;
    auto operator=(move_constructible_if const&)
    -> move_constructible_if&
    = default;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
