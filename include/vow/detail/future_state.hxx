#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_STATE
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_STATE

#include <vow/detail/mutex.hxx>
#include <vow/result.hxx>

#include <mutex>

namespace vow {
    template <typename Value>
    class promise;
}

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
struct future_base;

template <typename Value>
struct future_state
{
    detail::mutex& mutex;
    vow::promise<Value>*& promise;
    void (&dispatch)(future_base<Value>& future,
                     std::unique_lock<detail::mutex>& lock,
                     vow::result<Value>&& result);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
