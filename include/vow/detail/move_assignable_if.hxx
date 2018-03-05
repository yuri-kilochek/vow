#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_MOVE_ASSIGNABLE_IF
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_MOVE_ASSIGNABLE_IF

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <bool>
struct move_assignable_if {};

template <>
struct move_assignable_if<false>
{
    move_assignable_if()
    = default;

    move_assignable_if(move_assignable_if&&)
    = default;
    move_assignable_if(move_assignable_if const&)
    = default;

    auto operator=(move_assignable_if&&)
    -> move_assignable_if&
    = delete;
    auto operator=(move_assignable_if const&)
    -> move_assignable_if&
    = default;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
