#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT_BASE
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_RESULT_BASE

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
struct result_base;

template <typename Type>
class is_derived_from_result_base
{
    template <typename Value>
    static
    auto test(result_base<Value>*)
    -> std::true_type;

    static
    auto test(...)
    -> std::false_type;

public:
    inline static constexpr
    auto value = decltype(test(std::declval<Type*>()))::value;
};

template <typename Type>
inline constexpr
auto is_derived_from_result_base_v = is_derived_from_result_base<Type>::value;

template <typename Value>
struct result_base
{
    template <typename... Args, std::enable_if_t<
        std::is_constructible_v<result_value_box<Value>, Args...>>*...>
    result_base(with_value_t, Args&&... args)
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
    result_base(with_exception_t, ExceptionPtr&& exception_ptr)
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
    result_base(with_exception_t, Exception&& exception)
    noexcept
    : result_base(with_exception, [&]{
          try {
              return std::make_exception_ptr(
                  std::forward<Exception>(exception));
          } catch (...) {
              return std::current_exception();
          }
      }())
    {}

    template <typename Exception, typename... Args>
    result_base(with_exception_of_type_t<Exception>, Args&&... args)
    noexcept
    : result_base(with_exception, [&]{
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
    result_base(Args&&... args)
    noexcept
    : result_base(with_value, std::forward<Args>(args)...)
    {}

    template <typename... Args, std::enable_if_t<
        std::is_constructible_v<result_value_box<Value>, Args...> &&
        !is_implicitly_constructible_v<result_value_box<Value>, Args...>>*...>
    explicit
    result_base(Args&&... args)
    noexcept
    : result_base(with_value, std::forward<Args>(args)...)
    {}

    template <typename AnyValue, std::enable_if_t<
        !is_derived_from_result_base_v<std::decay_t<AnyValue>> &&
        std::is_constructible_v<result_value_box<Value>, AnyValue> &&
        std::is_assignable_v<result_value_box<Value>&, AnyValue>>*...>
    auto operator=(AnyValue&& any_value)
    noexcept
    -> result_base&
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

    result_base(result_base&& other)
    noexcept
    { construct(std::move(other)); }

    result_base(result_base const& other)
    noexcept
    { construct(other); }

    template <typename Other,
              typename OtherValueBox = 
                  decltype((std::declval<Other>().value_box_)),
    std::enable_if_t<
        is_derived_from_result_base_v<std::decay_t<Other>> &&
        is_implicitly_constructible_v<result_value_box<Value>, OtherValueBox>
    >*...>
    result_base(Other&& other)
    noexcept
    { construct(std::forward<Other>(other)); }

    template <typename Other,
              typename OtherValueBox = 
                  decltype((std::declval<Other>().value_box_)),
    std::enable_if_t<
        is_derived_from_result_base_v<std::decay_t<Other>> &&
        std::is_constructible_v<result_value_box<Value>, OtherValueBox> &&
        !is_implicitly_constructible_v<result_value_box<Value>, OtherValueBox>
    >*...>
    explicit
    result_base(Other&& other)
    noexcept
    { construct(std::forward<Other>(other)); }

    auto operator=(result_base&& other)
    noexcept
    -> result_base&
    {
        assign(std::move(other));
        return *this;
    }

    auto operator=(result_base const& other)
    noexcept
    -> result_base&
    {
        assign(other);
        return *this;
    }

    template <typename Other,
              typename OtherValueBox = 
                  decltype((std::declval<Other>().value_box_)),
    std::enable_if_t<
        is_derived_from_result_base_v<std::decay_t<Other>> &&
        std::is_constructible_v<result_value_box<Value>, OtherValueBox> &&
        std::is_assignable_v<result_value_box<Value>&, OtherValueBox>>*...>
    auto operator=(Other&& other)
    noexcept
    -> result_base&
    {
        assign(std::forward<Other>(other));
        return *this;
    }

    ~result_base()
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
    template <typename OtherValue>
    friend class result_base;

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
