#ifndef VOW_DETAIL_INCLUDE_GUARD_WITH_EXCEPTION
#define VOW_DETAIL_INCLUDE_GUARD_WITH_EXCEPTION

namespace vow {
///////////////////////////////////////////////////////////////////////////////

struct with_exception_t
{
    explicit
    with_exception_t()
    = default;
};

inline constexpr
with_exception_t with_exception{};

template <typename Exception>
struct with_exception_of_type_t
{
    explicit
    with_exception_of_type_t()
    = default;
};

template <typename Exception>
inline constexpr
with_exception_of_type_t<Exception> with_exception_of_type{};

///////////////////////////////////////////////////////////////////////////////
}

#endif
