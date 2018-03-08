#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT_IMPL
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT_IMPL

#include <vow/with_value.hxx>
#include <vow/with_exception.hxx>
#include <vow/detail/is_implicitly_constructible.hxx>
#include <vow/detail/result_value_box.hxx>

#include <type_traits>
#include <utility>
#include <new>
#include <exception>
#include <memory>

namespace vow::detail {
///////////////////////////////////////////////////////////////////////////////

template <typename Value>
struct result_impl
{
    template <typename... Args, std::enable_if_t<
        std::is_constructible_v<result_value_box<Value>, Args...>>*...>
    result_impl(with_value_t, Args&&... args)
    noexcept
    {
        try {
            new(&value_box_) decltype(value_box_)(std::forward<Args>(args)...);
            has_exception_ = false;
        } catch (...) {
            new(&exception_ptr_)
                decltype(exception_ptr_)(std::current_exception());
            has_exception_ = true;
        }
    }

    template <typename ExceptionPtr, std::enable_if_t<
        std::is_constructible_v<std::exception_ptr, ExceptionPtr>>*...>
    result_impl(with_exception_t, ExceptionPtr&& exception_ptr)
    noexcept
    {
        try {
            new(&exception_ptr_) decltype(exception_ptr_)(
                std::forward<ExceptionPtr>(exception_ptr));
        } catch (...) {
            new(&exception_ptr_) 
                decltype(exception_ptr_)(std::current_exception());
        }
        has_exception_ = false;
    }

    template <typename Exception, std::enable_if_t<
        !std::is_constructible_v<std::exception_ptr, Exception>>*...>
    result_impl(with_exception_t, Exception&& exception)
    noexcept
    : result_impl(with_exception, [&]{
          try {
              return std::make_exception_ptr(
                  std::forward<Exception>(exception));
          } catch (...) {
              return std::current_exception();
          }
      }())
    {}

    template <typename Exception, typename... Args>
    result_impl(with_exception_of_type_t<Exception>, Args&&... args)
    noexcept
    : result_impl(with_exception, [&]{
          try {
              return std::make_exception_ptr(
                  Exception(std::forward<Args>(args)...));
          } catch (...) {
              return std::current_exception();
          }
      }())
    {}

    template <typename... Args, std::enable_if_t<
        is_implicitly_constructible_v<result_value_box<Value>, Args...>>*...>
    result_impl(Args&&... args)
    noexcept
    : result_impl(with_value, std::forward<Args>(args)...)
    {}

    template <typename... Args, std::enable_if_t<
        std::is_constructible_v<result_value_box<Value>, Args...> &&
        !is_implicitly_constructible_v<result_value_box<Value>, Args...>>*...>
    explicit
    result_impl(Args&&... args)
    noexcept
    : result_impl(with_value, std::forward<Args>(args)...)
    {}

    template <typename AnyValue, std::enable_if_t<
        std::is_constructible_v<result_value_box<Value>, AnyValue> &&
        std::is_assignable_v<result_value_box<Value>&, AnyValue>>*...>
    auto operator=(AnyValue&& any_value)
    noexcept
    -> result_impl&
    {
        if (has_exception_) {
            std::destroy_at(&exception_ptr_);
            try {
                new(&value_box_) 
                    decltype(value_box_)(std::forward<AnyValue>(any_value));
                has_exception_ = false;
            } catch (...) {
                new(&exception_ptr_)
                    decltype(exception_ptr_)(std::current_exception());
            }
        } else {
            try {
                value_box_ = std::forward<AnyValue>(any_value);
            } catch (...) {
                std::destroy_at(&value_box_);
                new(&exception_ptr_)
                    decltype(exception_ptr_)(std::current_exception());
                has_exception_ = true;
            }
        }
        return *this;
    }

    result_impl(result_impl&& other)
    noexcept
    { construct(std::move(other)); }

    result_impl(result_impl const& other)
    noexcept
    { construct(other); }

    auto operator=(result_impl&& other)
    noexcept
    -> result_impl&
    {
        assign(std::move(other));
        return *this;
    }

    auto operator=(result_impl const& other)
    noexcept
    -> result_impl&
    {
        assign(other);
        return *this;
    }

    ~result_impl()
    {
        if (has_exception_) {
            std::destroy_at(&exception_ptr_);
        } else {
            std::destroy_at(&value_box_);
        }
    }

    auto get()
    -> std::add_rvalue_reference_t<Value>
    {
        if (has_exception_) {
            std::rethrow_exception(std::move(exception_ptr_));
        }
        return value_box_.get();
    }

private:
    union {
        result_value_box<Value> value_box_;
        std::exception_ptr exception_ptr_;
    };
    bool has_exception_;

    template <typename Other>
    void construct(Other&& other)
    noexcept
    {
        using ValueBox = decltype(value_box_);
        using OtherValueBox = decltype((std::declval<Other>().value_box_));
        if constexpr (std::is_constructible_v<ValueBox, OtherValueBox>) {
            if (other.has_exception_) {
                new(&exception_ptr_) decltype(exception_ptr_)(
                    std::forward<Other>(other).exception_ptr_);
                has_exception_ = true;
            } else {
                try {
                    new(&value_box_) decltype(value_box_)(
                        std::forward<Other>(other).value_box_);
                    has_exception_ = false;
                } catch (...) {
                    new(&exception_ptr_) 
                        decltype(exception_ptr_)(std::current_exception());
                    has_exception_ = true;
                }
            }
        }
    }

    template <typename Other>
    void assign(Other&& other)
    noexcept
    {
        using ValueBox = decltype(value_box_);
        using OtherValueBox = decltype((std::declval<Other>().value_box_));
        if constexpr (std::is_constructible_v<ValueBox, OtherValueBox> &&
                      std::is_assignable_v<ValueBox&, OtherValueBox>)
        {
            if (has_exception_) {
                if (other.has_exception_) {
                    exception_ptr_ = std::forward<Other>(other).exception_ptr_;
                } else {
                    std::destroy_at(&exception_ptr_);
                    try {
                        new(&value_box_) decltype(value_box_)(
                            std::forward<Other>(other).value_box_);
                        has_exception_ = false;
                    } catch (...) {
                        new(&exception_ptr_)
                            decltype(exception_ptr_)(std::current_exception());
                    }
                }
            } else {
                if (other.has_exception_) {
                    std::destroy_at(&value_box_);
                    new(&exception_ptr_) decltype(exception_ptr_)(
                        std::forward<Other>(other).exception_ptr_);
                    has_exception_ = true;
                } else {
                    try {
                        value_box_ = std::forward<Other>(other).value_box_;
                    } catch (...) {
                        std::destroy_at(&value_box_);
                        new(&exception_ptr_)
                            decltype(exception_ptr_)(std::current_exception());
                        has_exception_ = true;
                    }
                }
            }
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
