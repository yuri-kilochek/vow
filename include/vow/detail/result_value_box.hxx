#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT_VALUE_BOX
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT_VALUE_BOX

#include <vow/detail/is_implicitly_constructible.hxx>

#include <type_traits>
#include <utility>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Value, typename = void>
struct result_value_box;

template <typename Type>
struct is_result_value_box
: std::false_type
{};

template <typename Value, typename Dummy>
struct is_result_value_box<result_value_box<Value, Dummy>>
: std::true_type
{};

template <typename Type>
inline constexpr
auto is_result_value_box_v = is_result_value_box<Type>::value;

template <typename Value, typename>
struct result_value_box
{
    template <typename... Args, std::enable_if_t<
        is_implicitly_constructible_v<Value, Args...>>*...>
    result_value_box(Args&&... args)
    : value_(std::forward<Args>(args)...)
    {}

    template <typename... Args, std::enable_if_t<
        std::is_constructible_v<Value, Args...> &&
        !is_implicitly_constructible_v<Value, Args...>>*...>
    explicit
    result_value_box(Args&&... args)
    : value_(std::forward<Args>(args)...)
    {}

    template <typename AnyValue, std::enable_if_t<
        !is_result_value_box_v<std::decay_t<AnyValue>> &&
        std::is_assignable_v<Value&, AnyValue>>*...>
    auto operator=(AnyValue&& any_value)
    -> result_value_box&
    {
        value_ = std::forward<AnyValue>(any_value);
        return *this;
    }

    template <typename Other,
              typename OtherValue = decltype((std::declval<Other>().value_)),
    std::enable_if_t<
        is_result_value_box_v<std::decay_t<Other>> &&
        is_implicitly_constructible_v<Value, OtherValue>>*...>
    result_value_box(Other&& other)
    : value_(std::forward<Other>(other).value_)
    {}

    template <typename Other,
              typename OtherValue = decltype((std::declval<Other>().value_)),
    std::enable_if_t<
        is_result_value_box_v<std::decay_t<Other>> &&
        std::is_constructible_v<Value, OtherValue> &&
        !is_implicitly_constructible_v<Value, OtherValue>>*...>
    explicit
    result_value_box(Other&& other)
    : value_(std::forward<Other>(other).value_)
    {}

    template <typename Other,
              typename OtherValue = decltype((std::declval<Other>().value_)),
    std::enable_if_t<
        is_result_value_box_v<std::decay_t<Other>> &&
        std::is_assignable_v<Value&, OtherValue>>*...>
    auto operator=(Other&& other)
    -> result_value_box&
    {
        value_ = std::forward<Other>(other).value_;
        return *this;
    }

    template <typename OtherValue, std::enable_if_t<
        std::is_void_v<OtherValue> &&
        is_implicitly_constructible_v<Value>>*...>
    result_value_box(result_value_box<OtherValue>)
    : value_()
    {}

    template <typename OtherValue, std::enable_if_t<
        std::is_void_v<OtherValue> &&
        std::is_constructible_v<Value> &&
        !is_implicitly_constructible_v<Value>>*...>
    explicit
    result_value_box(result_value_box<OtherValue>)
    : value_()
    {}

    template <typename OtherValue, std::enable_if_t<
        std::is_void_v<OtherValue> &&
        std::is_constructible_v<Value> && 
        std::is_move_assignable_v<Value>>*...>
    auto operator=(result_value_box<OtherValue>)
    -> result_value_box&
    {
        value_ = Value();
        return *this;
    }

    auto get() 
    -> Value&&
    { return std::move(value_); }

private:
    template <typename OtherValue, typename>
    friend class result_value_box;

    Value value_;
};

template <typename Value>
struct result_value_box<Value, std::enable_if_t<std::is_void_v<Value>>>
{
    template <typename... Args>
    result_value_box(Args&&...)
    {}

    template <typename Arg>
    auto operator=(Arg&&)
    -> result_value_box&
    { return *this; }

    void get()
    {}
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
