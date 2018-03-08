#ifndef VOW_DETAIL_INCLUDE_GUARD_RESULT
#define VOW_DETAIL_INCLUDE_GUARD_RESULT

#include <vow/detail/result_impl.hxx>
#include <vow/detail/result_value_box.hxx>
#include <vow/detail/move_constructible_if.hxx>
#include <vow/detail/copy_constructible_if.hxx>
#include <vow/detail/move_assignable_if.hxx>
#include <vow/detail/copy_assignable_if.hxx>

#include <type_traits>
#include <functional>

namespace vow {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
struct result
: private detail::result_impl<Value>
, private detail::move_constructible_if<
    std::is_move_constructible_v<detail::result_value_box<Value>>>
, private detail::copy_constructible_if<
    std::is_copy_constructible_v<detail::result_value_box<Value>>>
, private detail::move_assignable_if<
    std::is_move_constructible_v<detail::result_value_box<Value>> &&
    std::is_move_assignable_v<detail::result_value_box<Value>>>
, private detail::copy_assignable_if<
    std::is_copy_constructible_v<detail::result_value_box<Value>> &&
    std::is_copy_assignable_v<detail::result_value_box<Value>>>
{
    using detail::result_impl<Value>::result_impl;
    using detail::result_impl<Value>::operator=;
    using detail::result_impl<Value>::get;
};

result()
-> result<void>;

template <typename Value>
result(Value&&)
-> result<std::decay_t<Value>>;

template <typename Value>
result(with_value_t, Value&&)
-> result<std::decay_t<Value>>;

template <typename Value>
result(std::reference_wrapper<Value>)
-> result<Value&>;

template <typename Value>
result(with_value_t, std::reference_wrapper<Value>)
-> result<Value&>;

///////////////////////////////////////////////////////////////////////////////
}

#endif
