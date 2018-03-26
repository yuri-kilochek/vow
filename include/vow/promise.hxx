#ifndef VOW_DETAIL_INCLUDE_GUARD_PROMISE
#define VOW_DETAIL_INCLUDE_GUARD_PROMISE

#include <vow/detail/future_base.hxx>
#include <vow/detail/dispatcher.hxx>
#include <vow/result.hxx>
#include <vow/detail/mutex.hxx>
#include <vow/detail/promise_status.hxx>
#include <vow/detail/with_lock.hxx>
#include <vow/abandoned.hxx>

#include <utility>
#include <type_traits>
#include <mutex>
#include <new>
#include <memory>
#include <exception>

namespace vow {
///////////////////////////////////////////////////////////////////////////////

template <typename Value, typename... Fns>
class future;

template <typename Value>
struct promise
{
    promise()
    noexcept
    : status_(detail::promise_status::unlinked)
    {}

    promise(promise&& other)
    noexcept
    { construct(std::move(other)); }

    auto operator=(promise&& other)
    noexcept
    -> promise&
    {
        if (this != &other) {
            destroy();
            construct(std::move(other));
        }
        return *this;
    }

    ~promise()
    { destroy(); }

    template <typename... Args, std::enable_if_t<
        std::is_constructible_v<result<Value>, Args...>>*...>
    void operator()(Args&&... args)
    noexcept
    { (*this)(result<Value>(std::forward<Args>(args)...)); }

    void operator()(vow::result<Value>&& result)
    noexcept
    {
        std::unique_lock lock{mutex_};
        (*this)(detail::with_lock, lock, std::move(result));
    }

private:
    template <typename Value_, typename... Fns>
    friend class future;

    union {
        detail::future_base<Value>* future_;
        detail::dispatcher<Value> dispatch_;
    };
    detail::mutex mutex_;
    detail::promise_status status_;

    void construct(promise&& other)
    noexcept
    {
        std::unique_lock other_lock{other.mutex_};
        switch (other.status_) {
            case detail::promise_status::unlinked: {
                other_lock.unlock();
                status_ = detail::promise_status::unlinked;
                break;
            }
            case detail::promise_status::linked: {
                new(&future_) decltype(future_)(std::move(other.future_));
                status_ = detail::promise_status::linked;
                std::destroy_at(&other.future_);
                other.status_ = detail::promise_status::unlinked;
                auto future_state = future_->get_state();
                other_lock.unlock();
                future_state.promise = this;
                break;
            }
            case detail::promise_status::ready: {
                other_lock.unlock();
                new(&dispatch_) 
                    decltype(dispatch_)(std::move(other.dispatch_));
                status_ = detail::promise_status::ready;
                std::destroy_at(&other.dispatch_);
                other.status_ = detail::promise_status::unlinked;
                break;
            }
        }
    }

    void destroy()
    noexcept
    {
        std::unique_lock lock{mutex_};
        switch (status_) {
            case detail::promise_status::unlinked: {
                lock.unlock();
                break;
            }
            case detail::promise_status::linked:
            case detail::promise_status::ready: {
                (*this)(detail::with_lock, lock,
                        with_exception_of_type<abandoned>);
                break;
            }
        }
    }

    void operator()(detail::with_lock_t, std::unique_lock<detail::mutex>& lock,
                    vow::result<Value>&& result)
    noexcept
    {
        switch (status_) {
            case detail::promise_status::unlinked: {
                lock.unlock();
                std::terminate();
                break;
            }
            case detail::promise_status::linked: {
                auto future = std::move(future_);
                std::destroy_at(&future_);
                status_ = detail::promise_status::unlinked;
                auto future_state = future_->get_state();
                lock.unlock();
                future_state.dispatch(*future, future_state.lock,
                                      std::move(result));
                break;
            }
            case detail::promise_status::ready: {
                lock.unlock();
                auto dispatch = std::move(dispatch_);
                std::destroy_at(&dispatch_);
                status_ = detail::promise_status::unlinked;
                dispatch(std::move(result));
                break;
            }
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
