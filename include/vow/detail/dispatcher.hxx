#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_DISPATCHER
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_DISPATCHER

#include <vow/result.hxx>
#include <vow/detail/dispatcher_continuation.hxx>

#include <utility>
#include <type_traits>
#include <memory>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
class dispatcher;

template <typename Value>
struct dispatcher_impl_base
{
    virtual
    ~dispatcher_impl_base()
    = default;

    virtual
    auto invoke(vow::result<Value>&& result)
    noexcept
    -> dispatcher_continuation
    = 0;
};

template <typename Value, typename Fn>
struct dispatcher_impl
: dispatcher_impl_base<Value>
{
    template <typename Fn_>
    dispatcher_impl(Fn_&& fn)
    : fn_(std::forward<Fn_>(fn))
    {}

    auto invoke(vow::result<Value>&& result)
    noexcept
    -> dispatcher_continuation
    override final
    { return std::move(fn_)(std::move(result)); }

private:
    Fn fn_;
};

template <typename Value>
struct dispatcher
{
    template <typename Fn,
              typename Impl = dispatcher_impl<Value, std::decay_t<Fn>>>
    dispatcher(Fn&& fn)
    : impl_(std::make_unique<Impl>(std::forward<Fn>(fn)))
    {}

    void operator()(vow::result<Value>&& result)
    noexcept
    {
        auto continuation = 
            std::exchange(impl_, {})->invoke(std::move(result));
        while (continuation) { continuation = continuation(); }
    }

private:
    std::unique_ptr<dispatcher_impl_base<Value>> impl_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
