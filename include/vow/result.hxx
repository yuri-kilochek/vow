#ifndef VOW_DETAIL_INCLUDE_GUARD_RESULT
#define VOW_DETAIL_INCLUDE_GUARD_RESULT

#include <vow/detail/result_base.hxx>
#include <vow/detail/result_value_box.hxx>
#include <vow/detail/move_constructible_if.hxx>
#include <vow/detail/copy_constructible_if.hxx>
#include <vow/detail/move_assignable_if.hxx>
#include <vow/detail/copy_assignable_if.hxx>

#include <type_traits>

namespace vow {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
class result
: detail::result_base<Value>
, detail::move_constructible_if<
    std::is_move_constructible_v<detail::result_value_box<Value>>>
, detail::copy_constructible_if<
    std::is_copy_constructible_v<detail::result_value_box<Value>>>
, detail::move_assignable_if<
    std::is_move_constructible_v<detail::result_value_box<Value>> &&
    std::is_move_assignable_v<detail::result_value_box<Value>>>
, detail::copy_assignable_if<
    std::is_copy_constructible_v<detail::result_value_box<Value>> &&
    std::is_copy_assignable_v<detail::result_value_box<Value>>>
{
    friend class detail::is_derived_from_result_base<result>;

    template <typename AnyValue>
    friend class detail::result_base;

public:
    using detail::result_base<Value>::result_base;
    using detail::result_base<Value>::operator=;
    using detail::result_base<Value>::get;
};

result()
-> result<void>;

template <typename Value>
result(Value&&)
-> result<std::decay_t<Value>>;

///////////////////////////////////////////////////////////////////////////////
}

#endif
