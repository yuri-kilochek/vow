#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_BASE
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_FUTURE_BASE

#include <vow/detail/condvar.hxx>
#include <vow/result.hxx>
#include <vow/detail/mutex.hxx>
#include <vow/detail/future_status.hxx>

namespace vow {
    template <typename Value>
    class promise;
}

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
struct future_base
{

protected:
    friend class promise<Value>;

    union {
        promise<Value>* promise_;
        condvar* condvar_;
        result<Value> result_;
    };
    mutex mutex_;
    future_status status_;

    virtual
    void dispatch()
    noexcept
    = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
