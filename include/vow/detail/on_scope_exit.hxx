#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_ON_SCOPE_EXIT
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_ON_SCOPE_EXIT

#include <vow/detail/unique_name.hxx>

#include <type_traits>
#include <utility>
#include <functional>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Action>
struct on_scope_exit {
    template <typename Action_>
    on_scope_exit(Action_&& action)
    noexcept(std::is_nothrow_constructible_v<Action, Action_>)
    : action_(std::forward<Action_>(action))
    {}

    ~on_scope_exit()
    noexcept(std::is_nothrow_invocable_v<Action>)
    { std::invoke(action_); }

private:
    Action action_;
};

template <typename Action>
on_scope_exit(Action&&)
-> on_scope_exit<std::decay_t<Action>>;

///////////////////////////////////////////////////////////////////////////////
}

#define VOW_DETAIL_ON_SCOPE_EXIT \
    ::vow::detail::on_scope_exit VOW_DETAIL_UNIQUE_NAME(on_scope_exit) =

#endif
