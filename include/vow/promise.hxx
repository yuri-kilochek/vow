#ifndef VOW_DETAIL_INCLUDE_GUARD_PROMISE
#define VOW_DETAIL_INCLUDE_GUARD_PROMISE

#include <vow/detail/future_base.hxx>
#include <vow/detail/dispatcher.hxx>
#include <vow/result.hxx>
#include <vow/detail/mutex.hxx>
#include <vow/detail/promise_status.hxx>

namespace vow {
///////////////////////////////////////////////////////////////////////////////

template <typename Value, typename... Fns>
class future;

template <typename Value>
struct promise
{
    promise() {}
    ~promise() {}

private:
    friend class detail::future_base<Value>;

    union {
        detail::future_base<Value>* future_;
        detail::dispatcher<Value> dispatcher_;
    };
    detail::mutex mutex_;
    detail::promise_status status_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
