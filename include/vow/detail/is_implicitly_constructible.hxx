#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_IS_IMPLICITLY_CONSTRUCTIBLE
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_IS_IMPLICITLY_CONSTRUCTIBLE

#include <utility>
#include <type_traits>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Type, typename... Args>
class is_implicitly_constructible
{
    template <typename Type_>
    static
    void indirect(Type_);

    template <typename Type_, typename... Args_, typename =
        decltype(indirect<Type_>({std::declval<Args_>()...}))>
    static auto test(int)
        -> std::true_type;
    
    template <typename Type_, typename... Args_>
    static auto test(...)
        -> std::false_type;

public:
    inline static constexpr
    auto value = decltype(test<Type, Args...>(0))::value;
};

template <typename Type, typename Arg>
struct is_implicitly_constructible<Type, Arg>
: std::is_convertible<Arg, Type>
{};

template <typename Type, typename... Args>
inline constexpr
auto is_implicitly_constructible_v = 
    is_implicitly_constructible<Type, Args...>::value;

///////////////////////////////////////////////////////////////////////////////
}

#endif
