#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_CONDVAR
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_CONDVAR

#include <condition_variable>
#include <chrono>
#include <utility>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

using condvar = std::condition_variable_any;

template <typename Lock, typename Representation, typename Period,
          typename Predicate>
auto wait(detail::condvar& condvar, Lock& lock,
          std::chrono::duration<Representation, Period> const& duration,
          Predicate&& predicate)
-> bool
{
    return condvar.wait_for(lock, duration,
                            std::forward<Predicate>(predicate));
}

template <typename Lock, typename Clock, typename Duration, typename Predicate>
auto wait(detail::condvar& condvar, Lock& lock,
          std::chrono::time_point<Clock, Duration> const& time_point,
          Predicate&& predicate)
-> bool
{
    return condvar.wait_until(lock, time_point,
                              std::forward<Predicate>(predicate));
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
