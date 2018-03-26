#ifndef VOW_DETAIL_INCLUDE_GUARD_PROMISE
#define VOW_DETAIL_INCLUDE_GUARD_PROMISE

#include <vow/result.hxx>
#include <vow/detail/with_lock.hxx>
#include <vow/detail/once_fn.hxx>
#include <vow/detail/mutex.hxx>
#include <vow/detail/promise_status.hxx>
#include <vow/abandoned.hxx>
#include <vow/detail/future_status.hxx>

#include <utility>
#include <type_traits>
#include <mutex>
#include <new>
#include <memory>
#include <exception>

namespace vow {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
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
    {
        std::unique_lock lock{mutex_};
        (*this)(detail::with_lock, lock, std::forward<Args>(args)...);
    }

private:
    friend class future<Value>;

    union {
        future<Value>* future_;
        detail::once_fn<void(result<Value>&&)> dispatch_;
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
                std::unique_lock future_lock{future_->mutex_};
                other_lock.unlock();
                future_->promise_ = this;
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

    template <typename... Args, std::enable_if_t<
        std::is_constructible_v<result<Value>, Args...>>*...>
    void operator()(detail::with_lock_t, std::unique_lock<detail::mutex>& lock,
                    Args&&... args)
    noexcept
    {
        switch (status_) {
            case detail::promise_status::unlinked: {
                std::terminate();
            }
            case detail::promise_status::linked: {
                auto future = std::move(future_);
                std::destroy_at(&future_);
                status_ = detail::promise_status::unlinked;
                std::unique_lock future_lock{future->mutex_};
                lock.unlock();
                switch (future->status_) {
                    case detail::future_status::unlinked: {
                        assert(false);
                        break;
                    }
                    case detail::future_status::linked: {
                        std::destroy_at(future->promise_);
                        new(&future->result_) decltype(future->result_)(
                            std::forward<Args>(args)...);
                        future->status_ = detail::future_status::ready;
                        break;
                    }
                    case detail::future_status::waiting: {
                        auto condvar = std::move(future->condvar_);
                        std::destroy_at(future->condvar_);
                        new(&future->result_) decltype(future->result_)(
                            std::forward<Args>(args)...);
                        future->status_ = detail::future_status::ready;
                        condvar->notify_one();
                        break;
                    }
                    case detail::future_status::ready: {
                        assert(false);
                        break;
                    }
                }
                break;
            }
            case detail::promise_status::ready: {
                lock.unlock();
                auto dispatch = std::move(dispatch_);
                std::destroy_at(&dispatch_);
                status_ = detail::promise_status::unlinked;
                dispatch(result<Value>(std::forward<Args>(args)...));
                break;
            }
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
