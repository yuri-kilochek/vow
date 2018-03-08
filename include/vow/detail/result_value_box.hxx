#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT_VALUE_BOX
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT_VALUE_BOX

#include <vow/detail/is_implicitly_constructible.hxx>

#include <type_traits>
#include <utility>
#include <functional>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Value, typename = void>
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
        std::is_assignable_v<Value&, AnyValue>>*...>
    auto operator=(AnyValue&& any_value)
    -> result_value_box&
    {
        value_ = std::forward<AnyValue>(any_value);
        return *this;
    }

    auto get()
    -> Value&&
    { return std::move(value_); }

private:
    Value value_;
};

template <typename Value>
struct result_value_box<Value&>
{
    template <typename AnyValue, std::enable_if_t<
        is_implicitly_constructible_v<Value*, AnyValue*>>*...>
    result_value_box(std::reference_wrapper<AnyValue> any_ref)
    : ptr_(std::addressof(any_ref.get()))
    {}

    template <typename AnyValue, std::enable_if_t<
        std::is_constructible_v<Value*, AnyValue*> &&
        !is_implicitly_constructible_v<Value*, AnyValue*>>*...>
    explicit
    result_value_box(std::reference_wrapper<AnyValue> any_ref)
    : ptr_(std::addressof(any_ref.get()))
    {}

    template <typename AnyValue, std::enable_if_t<
        std::is_assignable_v<Value*, AnyValue*>>*...>
    auto operator=(std::reference_wrapper<AnyValue> any_ref)
    -> result_value_box&
    {
        ptr_ = std::addressof(any_ref.get());
        return *this;
    }

    auto get() 
    -> Value&
    { return *ptr_; }

private:
    Value* ptr_;
};

template <typename Value>
struct result_value_box<Value, std::enable_if_t<std::is_void_v<Value>>>
{
    void get()
    {}
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
