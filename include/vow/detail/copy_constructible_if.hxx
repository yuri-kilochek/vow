#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_COPY_CONSTRUCTIBLE_IF
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_COPY_CONSTRUCTIBLE_IF

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <bool>
struct copy_constructible_if {};

template <>
struct copy_constructible_if<false>
{
    copy_constructible_if()
    = default;

    copy_constructible_if(copy_constructible_if&&)
    = default;
    copy_constructible_if(copy_constructible_if const&)
    = delete;

    auto operator=(copy_constructible_if&&)
    -> copy_constructible_if&
    = default;
    auto operator=(copy_constructible_if const&)
    -> copy_constructible_if&
    = default;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
