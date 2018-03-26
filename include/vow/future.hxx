#ifndef VOW_DETAIL_INCLUDE_GUARD_FUTURE
#define VOW_DETAIL_INCLUDE_GUARD_FUTURE

#include <vow/detail/future_status.hxx>
#include <vow/detail/condvar.hxx>
#include <vow/detail/on_scope_exit.hxx>
#include <vow/result.hxx>
#include <vow/detail/mutex.hxx>

#include <mutex>
#include <exception>

namespace vow {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
class promise;

template <typename Value>
struct future
{
    future() {}
    ~future() {}

    auto get_result()
    noexcept
    -> result<Value>
    {
        std::unique_lock lock{mutex_};
        switch (status_) {
            case detail::future_status::unlinked: {
                std::terminate();
            }
            case detail::future_status::linked: {
                detail::condvar condvar;
                std::destroy_at(&promise_);
                new(&condvar_) decltype(condvar_)(&condvar);
                status_ = detail::future_status::waiting;
                condvar.wait(lock, [&]{
                    return status_ == detail::future_status::ready;
                });
                lock.unlock();
                VOW_DETAIL_ON_SCOPE_EXIT [&]{
                    std::destroy_at(&result_);
                    status_ = detail::future_status::unlinked;
                };
                return std::move(result_);
            }
            case detail::future_status::waiting: {
                assert(false);
                break;
            }
            case detail::future_status::ready: {
                lock.unlock();
                VOW_DETAIL_ON_SCOPE_EXIT [&]{
                    std::destroy_at(&result_);
                    status_ = detail::future_status::unlinked;
                };
                return std::move(result_);
            }
        }
    }

    auto get()
    -> std::add_rvalue_reference_t<Value>
    { return get_result().get(); }

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
