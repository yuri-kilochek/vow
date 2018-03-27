#ifndef VOW_DETAIL_INCLUDE_GUARD_FUTURE
#define VOW_DETAIL_INCLUDE_GUARD_FUTURE

#include <vow/detail/future_status.hxx>
#include <vow/detail/condvar.hxx>
#include <vow/detail/on_scope_exit.hxx>
#include <vow/result.hxx>
#include <vow/detail/mutex.hxx>

#include <mutex>
#include <exception>
#include <memory>
#include <new>
#include <utility>
#include <cstdlib>
#include <optional>
#include <type_traits>

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
                std::destroy_at(&promise_);
                detail::condvar condvar;
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
                std::abort();
            }
            case detail::future_status::ready: {
                lock.unlock();
                VOW_DETAIL_ON_SCOPE_EXIT [&]{
                    std::destroy_at(&result_);
                    status_ = detail::future_status::unlinked;
                };
                return std::move(result_);
            }
            default: {
                std::abort();         
            }
        }
    }

    template <typename Timeout>
    auto get_result(Timeout const& timeout)
    noexcept
    -> std::optional<result<Value>>
    {
        std::unique_lock lock{mutex_};
        switch (status_) {
            case detail::future_status::unlinked: {
                std::terminate();
            }
            case detail::future_status::linked: {
                auto promise = std::move(promise_);
                std::destroy_at(&promise_);
                detail::condvar condvar;
                new(&condvar_) decltype(condvar_)(&condvar);
                status_ = detail::future_status::waiting;
                if (detail::wait(condvar, lock, timeout, [&]{
                    return status_ == detail::future_status::ready;
                })) {
                    lock.unlock();
                    VOW_DETAIL_ON_SCOPE_EXIT [&]{
                        std::destroy_at(&result_);
                        status_ = detail::future_status::unlinked;
                    };
                    return std::move(result_);
                }
                std::destroy_at(&condvar_);
                new(&promise_) decltype(promise_)(std::move(promise));
                status_ = detail::future_status::linked;
                return std::nullopt;
            }
            case detail::future_status::waiting: {
                std::abort();
            }
            case detail::future_status::ready: {
                lock.unlock();
                VOW_DETAIL_ON_SCOPE_EXIT [&]{
                    std::destroy_at(&result_);
                    status_ = detail::future_status::unlinked;
                };
                return std::move(result_);
            }
            default: {
                std::abort();         
            }
        }
    }

    auto get()
    -> std::add_rvalue_reference_t<Value>
    { return get_result().get(); }

    template <typename Timeout, typename Value_ = Value, std::enable_if_t<
        std::is_void_v<Value_>>*...>
    auto get(Timeout const& timeout)
    -> bool
    {
        if (auto result = get_result(timeout)) {
            result->get();
            return true;
        }
        return false;
    }

    template <typename Timeout, typename Value_ = Value, std::enable_if_t<
        std::is_lvalue_reference_v<Value_>>*...>
    auto get(Timeout const& timeout)
    -> std::add_pointer_t<Value_>
    {
        if (auto result = get_result(timeout)) {
            return std::addressof(result->get());
        }
        return nullptr;
    }

    template <typename Timeout, typename Value_ = Value, std::enable_if_t<
        std::is_object_v<Value_>>*...>
    auto get(Timeout const& timeout)
    -> std::optional<Value>
    {
        if (auto result = get_result(timeout)) {
            return result->get();
        }
        return std::nullopt;
    }

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
