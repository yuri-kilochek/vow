#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_DISPATCHER_CONTINUATION
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_DISPATCHER_CONTINUATION

#include <utility>
#include <type_traits>
#include <memory>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

class dispatcher_continuation;

struct dispatcher_continuation_impl_base
{
    virtual
    ~dispatcher_continuation_impl_base()
    = default;

    virtual
    auto invoke()
    noexcept
    -> dispatcher_continuation
    = 0;
};

template <typename Fn>
class dispatcher_continuation_impl;

struct dispatcher_continuation
{
    template <typename Fn,
              typename Impl = dispatcher_continuation_impl<std::decay_t<Fn>>>
    dispatcher_continuation(Fn&& fn)
    : impl_(std::make_unique<Impl>(std::forward<Fn>(fn)))
    {}

    dispatcher_continuation()
    = default;

    explicit
    operator bool() const
    noexcept
    { return !!impl_; }

    auto operator()()
    noexcept
    -> dispatcher_continuation
    { return std::exchange(impl_, {})->invoke(); }

private:
    std::unique_ptr<dispatcher_continuation_impl_base> impl_;
};

template <typename Fn>
struct dispatcher_continuation_impl
: dispatcher_continuation_impl_base
{
    template <typename Fn_>
    dispatcher_continuation_impl(Fn_&& fn)
    : fn_(std::forward<Fn_>(fn))
    {}

    auto invoke()
    noexcept
    -> dispatcher_continuation
    override final
    { return std::move(fn_)(); }

private:
    Fn fn_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
