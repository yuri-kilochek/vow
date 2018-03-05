#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_COPY_ASSIGNABLE_IF
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_COPY_ASSIGNABLE_IF

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <bool>
struct copy_assignable_if {};

template <>
struct copy_assignable_if<false>
{
    copy_assignable_if()
    = default;

    copy_assignable_if(copy_assignable_if&&)
    = default;
    copy_assignable_if(copy_assignable_if const&)
    = default;

    auto operator=(copy_assignable_if&&)
    -> copy_assignable_if&
    = default;
    auto operator=(copy_assignable_if const&)
    -> copy_assignable_if&
    = delete;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
