#ifndef VOW_DETAIL_INCLUDE_GUARD_FUTURE
#define VOW_DETAIL_INCLUDE_GUARD_FUTURE

#include <vow/detail/condvar.hxx>
#include <vow/result.hxx>
#include <vow/detail/mutex.hxx>
#include <vow/detail/future_status.hxx>

namespace vow {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
class promise;

template <typename Value>
struct future
{
    future() {}
    ~future() {}

private:
    friend class promise<Value>;

    union {
        promise<Value>* promise_;
        detail::condvar* condvar_;
        result<Value> result_;
    };
    detail::mutex mutex_;
    detail::future_status status_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
